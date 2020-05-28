/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import * as vscode from 'vscode';
import * as nls from 'vscode-nls';
import { TypeScriptVersion, TypeScriptVersionProvider } from './versionProvider';
import { Disposable } from './dispose';

const localize = nls.loadMessageBundle();

const useWorkspaceTsdkStorageKey = 'typescript.useWorkspaceTsdk';

interface QuickPickItem extends vscode.QuickPickItem {
	run(): void;
}

export class TypeScriptVersionManager extends Disposable {

	private _currentVersion: TypeScriptVersion;

	public constructor(
		private readonly versionProvider: TypeScriptVersionProvider,
		private readonly workspaceState: vscode.Memento
	) {
		super();

		this._currentVersion = this.versionProvider.defaultVersion;

		if (this.useWorkspaceTsdkSetting) {
			const localVersion = this.versionProvider.localVersion;
			if (localVersion) {
				this._currentVersion = localVersion;
			}
		}
	}

	private readonly _onDidPickNewVersion = this._register(new vscode.EventEmitter<void>());
	public readonly onDidPickNewVersion = this._onDidPickNewVersion.event;

	public get currentVersion(): TypeScriptVersion {
		return this._currentVersion;
	}

	public reset(): void {
		this._currentVersion = this.versionProvider.bundledVersion;
	}

	public async promptUserForVersion(): Promise<void> {
		const selected = await vscode.window.showQuickPick<QuickPickItem>([
			this.getBundledPickItem(),
			...this.getLocalPickItems(),
			LearnMorePickItem,
		], {
			placeHolder: localize(
				'selectTsVersion',
				"Select the TypeScript version used for JavaScript and TypeScript language features"),
		});

		return selected?.run();
	}

	private getBundledPickItem(): QuickPickItem {
		const bundledVersion = this.versionProvider.defaultVersion;
		return {
			label: (!this.useWorkspaceTsdkSetting
				? '• '
				: '') + localize('useVSCodeVersionOption', "Use VS Code's Version"),
			description: bundledVersion.displayName,
			detail: bundledVersion.pathLabel,
			run: async () => {
				await this.workspaceState.update(useWorkspaceTsdkStorageKey, false);
				this.updateForPickedVersion(bundledVersion);
			},
		};
	}

	private getLocalPickItems(): QuickPickItem[] {
		return this.versionProvider.localVersions.map(version => {
			return {
				label: (this.useWorkspaceTsdkSetting && this.currentVersion.eq(version)
					? '• '
					: '') + localize('useWorkspaceVersionOption', "Use Workspace Version"),
				description: version.displayName,
				detail: version.pathLabel,
				run: async () => {
					await this.workspaceState.update(useWorkspaceTsdkStorageKey, true);
					const tsConfig = vscode.workspace.getConfiguration('typescript');
					await tsConfig.update('tsdk', version.pathLabel, false);
					this.updateForPickedVersion(version);
				},
			};
		});
	}

	private updateForPickedVersion(pickedVersion: TypeScriptVersion) {
		const oldVersion = this.currentVersion;
		this._currentVersion = pickedVersion;
		if (!oldVersion.eq(pickedVersion)) {
			this._onDidPickNewVersion.fire();
		}
	}

	private get useWorkspaceTsdkSetting(): boolean {
		return this.workspaceState.get<boolean>(useWorkspaceTsdkStorageKey, false);
	}
}

const LearnMorePickItem: QuickPickItem = {
	label: localize('learnMore', 'Learn more about managing TypeScript versions'),
	description: '',
	run: () => {
		vscode.env.openExternal(vscode.Uri.parse('https://go.microsoft.com/fwlink/?linkid=839919'));
	}
};
