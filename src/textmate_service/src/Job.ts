/*
 * JobManager.ts
 *
 * Entry for the external textmate tokenizer service
 */

export interface Job {
    priority: number
    execute: () => Job[]
}

export class JobManager {
    private _jobs: Job[] = []
    private _doNextJob() {
        let pendingJob = this._jobs.pop()

        if (pendingJob) {
            let moreJobs = pendingJob.execute() || []
            moreJobs.forEach(j => this.queueJob(j))
        }

        this._schedule()
    }
    private _schedule() {
        if (this._jobs.length > 0) {
            setTimeout(() => this._doNextJob(), 16)
        }
    }
    public queueJob(job: Job) {
        let jobsToQueue = [job]
        if (job.priority == 1) {
            jobsToQueue = job.execute()
        }

        this._jobs = [...jobsToQueue, ...this._jobs].sort((a, b) => b.priority - a.priority)

        this._schedule()
    }
}
