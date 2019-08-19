This extension allow to submit jobs to a DRMAA/Slurm cluster using drmaa library.
It was originally written for SGE, then migrated to Slurm (not guaranteed to work with SGE anymore, or any other system than Slurm)
It implements 3 basic php functions: qsub, qstat, qdel.
The original version for SGE is at https://gforge.inria.fr/projects/php4drm

# Installation

Download the latest version of the sources, cd in source dir, then launch:

```bash
./configure
make
```

phpdrmaa.so is created in modules subdir. Copy it in php modules dir:

```bash
cp modules/phpdrmaa.so /usr/lib/php/modules/
```

And load it in php.ini:

```bash
echo 'extension=phpdrmaa.so' > /etc/php.ini
```

And finally restart httpd.


# Configuring the extension

Some specific settings can be modified in php.ini:

'drmaa.stdlog' is the file or directory where stdout output from each job will be saved. Default is /tmp (for sge) or /tmp/job.o<jobid> (for slurm).
'drmaa.errorlog' is the file or directory where stderr output from each job will be saved. Default is /tmp/job.e<jobid>.
'drmaa.native' is the drmaa native specifications that will be applied to all jobs (for example to submit all jobs to a specific queue).

Examples:
drmaa.stdlog=:/home/genouest/bioinfo/drmaa_log/standard
drmaa.errorlog=:/home/genouest/bioinfo/drmaa_log/error
drmaa.native=-p fastjobs


# Submitting a job

To submit a job, you have to use the 'qsub' function:

```php
$jobid = qsub('echo foobar');
```

This will launch the command 'echo foobar' on a node of the cluster and return the associated job id.
If there is an error while submiting the job, the returned value is NULL.

You can give a name to the job:

```php
$jobid = qsub('echo foobar', 'myjob');
```

And you can add native drmaa specifications, like a queue for example:

```php
$jobid = qsub('echo foobar', 'myjob', '-q slowjobs');
```

In this case, the native specifications defined in the php.ini will be ignored

# Watching the status of a job

If you want to get the status of a job, you can use the 'qstat' function:

```php
$status = qstat($jobid);
```

This function returns an integer value on success:

```
    STATE_UNDETERMINED                  =  0; /* process status cannot be determined */
    STATE_WAITING                       = 16; /* job is queued and active */
    STATE_WAITING_SYSTEM_ON_HOLD        = 17; /* job is queued and in system hold */
    STATE_WAITING_USER_ON_HOLD          = 18; /* job is queued and in user hold */
    STATE_WAITING_USER_SYSTEM_ON_HOLD   = 19; /* job is queued and in user and system hold */
    STATE_RUNNING                       = 32; /* job is running */
    STATE_SYSTEM_SUSPENDED              = 33; /* job is system suspended */
    STATE_USER_SUSPENDED                = 34; /* job is user suspended */
    STATE_USER_SYSTEM_SUSPENDED         = 35; /* job is user and system suspended */
    STATE_FINISHED                      = 48; /* job finished normally */
    STATE_FAILED                        = 64; /* job finished, but failed */
```

On failure, STATE_UNDETERMINED is returned. Note that finished jobs can return STATE_UNDETERMINED.


# Terminating a job

If you want to stop a job, you can use the 'qdel' function:

```php
$success = qdel($jobid);
```

This function returns TRUE on success, FALSE otherwise.
