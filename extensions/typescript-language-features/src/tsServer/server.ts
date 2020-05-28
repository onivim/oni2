/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import * as fs from 'fs';
import * as stream from 'stream';
import * as vscode from 'vscode';
import type * as Proto from '../protocol';
import { ServerResponse, TypeScriptRequests } from '../typescriptService';
import { Disposable } from '../utils/dispose';
import { TelemetryReporter } from '../utils/telemetry';
import Tracer from '../utils/tracer';
import { TypeScriptVersion } from '../utils/versionProvider';
import { Reader } from '../utils/wireProtocol';
import { CallbackMap } from './callbackMap';
import { RequestItem, RequestQueue, RequestQueueingType } from './requestQueue';
import { TypeScriptServerError } from './serverError';

export interface OngoingRequestCanceller {
	tryCancelOngoingRequest(seq: number): boolean;
}

export class PipeRequestCanceller implements OngoingRequestCanceller {
	public constructor(
		private readonly _serverId: string,
		private readonly _cancellationPipeName: string | undefined,
		private readonly _tracer: Tracer,
	) { }

	public tryCancelOngoingRequest(seq: number): boolean {
		if (!this._cancellationPipeName) {
			return false;
		}
		this._tracer.logTrace(this._serverId, `TypeScript Server: trying to cancel ongoing request with sequence number ${seq}`);
		try {
			fs.writeFileSync(this._cancellationPipeName + seq, '');
		} catch {
			// noop
		}
		return true;
	}
}

export interface ITypeScriptServer {
	readonly onEvent: vscode.Event<Proto.Event>;
	readonly onExit: vscode.Event<any>;
	readonly onError: vscode.Event<any>;
	readonly onReaderError: vscode.Event<Error>;

	readonly tsServerLogFile: string | undefined;

	kill(): void;

	executeImpl(command: keyof TypeScriptRequests, args: any, executeInfo: { isAsync: boolean, token?: vscode.CancellationToken, expectsResult: false, lowPriority?: boolean }): undefined;
	executeImpl(command: keyof TypeScriptRequests, args: any, executeInfo: { isAsync: boolean, token?: vscode.CancellationToken, expectsResult: boolean, lowPriority?: boolean }): Promise<ServerResponse.Response<Proto.Response>>;
	executeImpl(command: keyof TypeScriptRequests, args: any, executeInfo: { isAsync: boolean, token?: vscode.CancellationToken, expectsResult: boolean, lowPriority?: boolean }): Promise<ServerResponse.Response<Proto.Response>> | undefined;

	dispose(): void;
}

export interface TsServerDelegate {
	onFatalError(command: string, error: Error): void;
}

export interface TsServerProcess {
	readonly stdout: stream.Readable;
	write(serverRequest: Proto.Request): void;

	on(name: 'exit', handler: (code: number | null) => void): void;
	on(name: 'error', handler: (error: Error) => void): void;

	kill(): void;
}

export class ProcessBasedTsServer extends Disposable implements ITypeScriptServer {
	private readonly _reader: Reader<Proto.Response>;
	private readonly _requestQueue = new RequestQueue();
	private readonly _callbacks = new CallbackMap<Proto.Response>();
	private readonly _pendingResponses = new Set<number>();

	constructor(
		private readonly _serverId: string,
		private readonly _process: TsServerProcess,
		private readonly _tsServerLogFile: string | undefined,
		private readonly _requestCanceller: OngoingRequestCanceller,
		private readonly _version: TypeScriptVersion,
		private readonly _telemetryReporter: TelemetryReporter,
		private readonly _tracer: Tracer,
	) {
		super();
		this._reader = this._register(new Reader<Proto.Response>(this._process.stdout!));
		this._reader.onData(msg => this.dispatchMessage(msg));

		this._process.on('exit', code => {
			this._onExit.fire(code);
			this._callbacks.destroy('server exited');
		});
		this._process.on('error', error => {
			this._onError.fire(error);
			this._callbacks.destroy('server errored');
		});
	}

	private readonly _onEvent = this._register(new vscode.EventEmitter<Proto.Event>());
	public readonly onEvent = this._onEvent.event;

	private readonly _onExit = this._register(new vscode.EventEmitter<any>());
	public readonly onExit = this._onExit.event;

	private readonly _onError = this._register(new vscode.EventEmitter<any>());
	public readonly onError = this._onError.event;

	public get onReaderError() { return this._reader.onError; }

	public get tsServerLogFile() { return this._tsServerLogFile; }

	private write(serverRequest: Proto.Request) {
		this._process.write(serverRequest);
	}

	public dispose() {
		super.dispose();
		this._callbacks.destroy('server disposed');
		this._pendingResponses.clear();
	}

	public kill() {
		this._process.kill();
	}

	private dispatchMessage(message: Proto.Message) {
		try {
			switch (message.type) {
				case 'response':
					this.dispatchResponse(message as Proto.Response);
					break;

				case 'event':
					const event = message as Proto.Event;
					if (event.event === 'requestCompleted') {
						const seq = (event as Proto.RequestCompletedEvent).body.request_seq;
						const callback = this._callbacks.fetch(seq);
						if (callback) {
							this._tracer.traceRequestCompleted(this._serverId, 'requestCompleted', seq, callback);
							callback.onSuccess(undefined);
						}
					} else {
						this._tracer.traceEvent(this._serverId, event);
						this._onEvent.fire(event);
					}
					break;

				default:
					throw new Error(`Unknown message type ${message.type} received`);
			}
		} finally {
			this.sendNextRequests();
		}
	}

	private tryCancelRequest(seq: number, command: string): boolean {
		try {
			if (this._requestQueue.tryDeletePendingRequest(seq)) {
				this.logTrace(`Canceled request with sequence number ${seq}`);
				return true;
			}

			if (this._requestCanceller.tryCancelOngoingRequest(seq)) {
				return true;
			}

			this.logTrace(`Tried to cancel request with sequence number ${seq}. But request got already delivered.`);
			return false;
		} finally {
			const callback = this.fetchCallback(seq);
			if (callback) {
				callback.onSuccess(new ServerResponse.Cancelled(`Cancelled request ${seq} - ${command}`));
			}
		}
	}

	private dispatchResponse(response: Proto.Response) {
		const callback = this.fetchCallback(response.request_seq);
		if (!callback) {
			return;
		}

		this._tracer.traceResponse(this._serverId, response, callback);
		if (response.success) {
			callback.onSuccess(response);
		} else if (response.message === 'No content available.') {
			// Special case where response itself is successful but there is not any data to return.
			callback.onSuccess(ServerResponse.NoContent);
		} else {
			callback.onError(TypeScriptServerError.create(this._serverId, this._version, response));
		}
	}

	public executeImpl(command: keyof TypeScriptRequests, args: any, executeInfo: { isAsync: boolean, token?: vscode.CancellationToken, expectsResult: false, lowPriority?: boolean }): undefined;
	public executeImpl(command: keyof TypeScriptRequests, args: any, executeInfo: { isAsync: boolean, token?: vscode.CancellationToken, expectsResult: boolean, lowPriority?: boolean }): Promise<ServerResponse.Response<Proto.Response>>;
	public executeImpl(command: keyof TypeScriptRequests, args: any, executeInfo: { isAsync: boolean, token?: vscode.CancellationToken, expectsResult: boolean, lowPriority?: boolean }): Promise<ServerResponse.Response<Proto.Response>> | undefined {
		const request = this._requestQueue.createRequest(command, args);
		const requestInfo: RequestItem = {
			request,
			expectsResponse: executeInfo.expectsResult,
			isAsync: executeInfo.isAsync,
			queueingType: ProcessBasedTsServer.getQueueingType(command, executeInfo.lowPriority)
		};
		let result: Promise<ServerResponse.Response<Proto.Response>> | undefined;
		if (executeInfo.expectsResult) {
			result = new Promise<ServerResponse.Response<Proto.Response>>((resolve, reject) => {
				this._callbacks.add(request.seq, { onSuccess: resolve, onError: reject, queuingStartTime: Date.now(), isAsync: executeInfo.isAsync }, executeInfo.isAsync);

				if (executeInfo.token) {
					executeInfo.token.onCancellationRequested(() => {
						this.tryCancelRequest(request.seq, command);
					});
				}
			}).catch((err: Error) => {
				if (err instanceof TypeScriptServerError) {
					if (!executeInfo.token || !executeInfo.token.isCancellationRequested) {
						/* __GDPR__
							"languageServiceErrorResponse" : {
								"${include}": [
									"${TypeScriptCommonProperties}",
									"${TypeScriptRequestErrorProperties}"
								]
							}
						*/
						this._telemetryReporter.logTelemetry('languageServiceErrorResponse', err.telemetry);
					}
				}

				throw err;
			});
		}

		this._requestQueue.enqueue(requestInfo);
		this.sendNextRequests();

		return result;
	}

	private sendNextRequests(): void {
		while (this._pendingResponses.size === 0 && this._requestQueue.length > 0) {
			const item = this._requestQueue.dequeue();
			if (item) {
				this.sendRequest(item);
			}
		}
	}

	private sendRequest(requestItem: RequestItem): void {
		const serverRequest = requestItem.request;
		this._tracer.traceRequest(this._serverId, serverRequest, requestItem.expectsResponse, this._requestQueue.length);

		if (requestItem.expectsResponse && !requestItem.isAsync) {
			this._pendingResponses.add(requestItem.request.seq);
		}

		try {
			this.write(serverRequest);
		} catch (err) {
			const callback = this.fetchCallback(serverRequest.seq);
			if (callback) {
				callback.onError(err);
			}
		}
	}

	private fetchCallback(seq: number) {
		const callback = this._callbacks.fetch(seq);
		if (!callback) {
			return undefined;
		}

		this._pendingResponses.delete(seq);
		return callback;
	}

	private logTrace(message: string) {
		this._tracer.logTrace(this._serverId, message);
	}

	private static readonly fenceCommands = new Set(['change', 'close', 'open', 'updateOpen']);

	private static getQueueingType(
		command: string,
		lowPriority?: boolean
	): RequestQueueingType {
		if (ProcessBasedTsServer.fenceCommands.has(command)) {
			return RequestQueueingType.Fence;
		}
		return lowPriority ? RequestQueueingType.LowPriority : RequestQueueingType.Normal;
	}
}


class RequestRouter {

	private static readonly sharedCommands = new Set<keyof TypeScriptRequests>([
		'change',
		'close',
		'open',
		'updateOpen',
		'configure',
		'configurePlugin',
	]);

	constructor(
		private readonly servers: ReadonlyArray<{ readonly server: ITypeScriptServer, readonly preferredCommands?: ReadonlySet<keyof TypeScriptRequests> }>,
		private readonly delegate: TsServerDelegate,
	) { }

	public execute(command: keyof TypeScriptRequests, args: any, executeInfo: { isAsync: boolean, token?: vscode.CancellationToken, expectsResult: boolean, lowPriority?: boolean }): Promise<ServerResponse.Response<Proto.Response>> | undefined {
		if (RequestRouter.sharedCommands.has(command)) {
			// Dispatch shared commands to all server but only return from first one one

			const requestStates: RequestState.State[] = this.servers.map(() => RequestState.Unresolved);

			// Also make sure we never cancel requests to just one server
			let token: vscode.CancellationToken | undefined = undefined;
			if (executeInfo.token) {
				const source = new vscode.CancellationTokenSource();
				executeInfo.token.onCancellationRequested(() => {
					if (requestStates.some(state => state === RequestState.Resolved)) {
						// Don't cancel.
						// One of the servers completed this request so we don't want to leave the other
						// in a different state.
						return;
					}
					source.cancel();
				});
				token = source.token;
			}

			let firstRequest: Promise<ServerResponse.Response<Proto.Response>> | undefined;

			for (let serverIndex = 0; serverIndex < this.servers.length; ++serverIndex) {
				const server = this.servers[serverIndex].server;

				const request = server.executeImpl(command, args, { ...executeInfo, token });
				if (serverIndex === 0) {
					firstRequest = request;
				}
				if (request) {
					request
						.then(result => {
							requestStates[serverIndex] = RequestState.Resolved;
							const erroredRequest = requestStates.find(state => state.type === RequestState.Type.Errored) as RequestState.Errored | undefined;
							if (erroredRequest) {
								// We've gone out of sync
								this.delegate.onFatalError(command, erroredRequest.err);
							}
							return result;
						}, err => {
							requestStates[serverIndex] = new RequestState.Errored(err);
							if (requestStates.some(state => state === RequestState.Resolved)) {
								// We've gone out of sync
								this.delegate.onFatalError(command, err);
							}
							throw err;
						});
				}
			}

			return firstRequest;
		}

		for (const { preferredCommands, server } of this.servers) {
			if (!preferredCommands || preferredCommands.has(command)) {
				return server.executeImpl(command, args, executeInfo);
			}
		}

		throw new Error(`Could not find server for command: '${command}'`);
	}
}


export class SyntaxRoutingTsServer extends Disposable implements ITypeScriptServer {

	private static readonly syntaxCommands = new Set<keyof TypeScriptRequests>([
		'navtree',
		'getOutliningSpans',
		'jsxClosingTag',
		'selectionRange',
		'format',
		'formatonkey',
		'docCommentTemplate',
	]);

	private readonly syntaxServer: ITypeScriptServer;
	private readonly semanticServer: ITypeScriptServer;
	private readonly router: RequestRouter;

	public constructor(
		servers: { syntax: ITypeScriptServer, semantic: ITypeScriptServer },
		delegate: TsServerDelegate,
	) {
		super();

		this.syntaxServer = servers.syntax;
		this.semanticServer = servers.semantic;

		this.router = new RequestRouter(
			[
				{ server: this.syntaxServer, preferredCommands: SyntaxRoutingTsServer.syntaxCommands },
				{ server: this.semanticServer, preferredCommands: undefined /* gets all other commands */ }
			],
			delegate);

		this._register(this.syntaxServer.onEvent(e => this._onEvent.fire(e)));
		this._register(this.semanticServer.onEvent(e => this._onEvent.fire(e)));

		this._register(this.semanticServer.onExit(e => {
			this._onExit.fire(e);
			this.syntaxServer.kill();
		}));
		this._register(this.semanticServer.onError(e => this._onError.fire(e)));
	}

	private readonly _onEvent = this._register(new vscode.EventEmitter<Proto.Event>());
	public readonly onEvent = this._onEvent.event;

	private readonly _onExit = this._register(new vscode.EventEmitter<any>());
	public readonly onExit = this._onExit.event;

	private readonly _onError = this._register(new vscode.EventEmitter<any>());
	public readonly onError = this._onError.event;

	public get onReaderError() { return this.semanticServer.onReaderError; }

	public get tsServerLogFile() { return this.semanticServer.tsServerLogFile; }

	public kill(): void {
		this.syntaxServer.kill();
		this.semanticServer.kill();
	}

	public executeImpl(command: keyof TypeScriptRequests, args: any, executeInfo: { isAsync: boolean, token?: vscode.CancellationToken, expectsResult: false, lowPriority?: boolean }): undefined;
	public executeImpl(command: keyof TypeScriptRequests, args: any, executeInfo: { isAsync: boolean, token?: vscode.CancellationToken, expectsResult: boolean, lowPriority?: boolean }): Promise<ServerResponse.Response<Proto.Response>>;
	public executeImpl(command: keyof TypeScriptRequests, args: any, executeInfo: { isAsync: boolean, token?: vscode.CancellationToken, expectsResult: boolean, lowPriority?: boolean }): Promise<ServerResponse.Response<Proto.Response>> | undefined {
		return this.router.execute(command, args, executeInfo);
	}
}


export class GetErrRoutingTsServer extends Disposable implements ITypeScriptServer {

	private static readonly diagnosticEvents = new Set([
		'configFileDiag',
		'syntaxDiag',
		'semanticDiag',
		'suggestionDiag'
	]);

	private readonly getErrServer: ITypeScriptServer;
	private readonly mainServer: ITypeScriptServer;
	private readonly router: RequestRouter;

	public constructor(
		servers: { getErr: ITypeScriptServer, primary: ITypeScriptServer },
		delegate: TsServerDelegate,
	) {
		super();

		this.getErrServer = servers.getErr;
		this.mainServer = servers.primary;

		this.router = new RequestRouter(
			[
				{ server: this.getErrServer, preferredCommands: new Set<keyof TypeScriptRequests>(['geterr', 'geterrForProject']) },
				{ server: this.mainServer, preferredCommands: undefined /* gets all other commands */ }
			],
			delegate);

		this._register(this.getErrServer.onEvent(e => {
			if (GetErrRoutingTsServer.diagnosticEvents.has(e.event)) {
				this._onEvent.fire(e);
			}
			// Ignore all other events
		}));
		this._register(this.mainServer.onEvent(e => {
			if (!GetErrRoutingTsServer.diagnosticEvents.has(e.event)) {
				this._onEvent.fire(e);
			}
			// Ignore all other events
		}));

		this._register(this.getErrServer.onError(e => this._onError.fire(e)));
		this._register(this.mainServer.onError(e => this._onError.fire(e)));

		this._register(this.mainServer.onExit(e => {
			this._onExit.fire(e);
			this.getErrServer.kill();
		}));
	}

	private readonly _onEvent = this._register(new vscode.EventEmitter<Proto.Event>());
	public readonly onEvent = this._onEvent.event;

	private readonly _onExit = this._register(new vscode.EventEmitter<any>());
	public readonly onExit = this._onExit.event;

	private readonly _onError = this._register(new vscode.EventEmitter<any>());
	public readonly onError = this._onError.event;

	public get onReaderError() { return this.mainServer.onReaderError; }

	public get tsServerLogFile() { return this.mainServer.tsServerLogFile; }

	public kill(): void {
		this.getErrServer.kill();
		this.mainServer.kill();
	}

	public executeImpl(command: keyof TypeScriptRequests, args: any, executeInfo: { isAsync: boolean, token?: vscode.CancellationToken, expectsResult: false, lowPriority?: boolean }): undefined;
	public executeImpl(command: keyof TypeScriptRequests, args: any, executeInfo: { isAsync: boolean, token?: vscode.CancellationToken, expectsResult: boolean, lowPriority?: boolean }): Promise<ServerResponse.Response<Proto.Response>>;
	public executeImpl(command: keyof TypeScriptRequests, args: any, executeInfo: { isAsync: boolean, token?: vscode.CancellationToken, expectsResult: boolean, lowPriority?: boolean }): Promise<ServerResponse.Response<Proto.Response>> | undefined {
		return this.router.execute(command, args, executeInfo);
	}
}


namespace RequestState {
	export const enum Type { Unresolved, Resolved, Errored }

	export const Unresolved = { type: Type.Unresolved } as const;

	export const Resolved = { type: Type.Resolved } as const;

	export class Errored {
		readonly type = Type.Errored;

		constructor(
			public readonly err: Error
		) { }
	}

	export type State = typeof Unresolved | typeof Resolved | Errored;
}
