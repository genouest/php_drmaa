/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2007 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header,v 1.16.2.1.2.1 2007/01/01 19:32:09 iliaa Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_drmaa.h"

#include "drmaa.h"

/* If you declare any globals in php_drmaa.h uncomment this:*/
ZEND_DECLARE_MODULE_GLOBALS(drmaa)


/* True global resources - no need for thread safety here */
static int le_drmaa;

/* {{{ drmaa_functions[]
 *
 * Every user visible function must have an entry in drmaa_functions[].
 */
zend_function_entry drmaa_functions[] = {
	PHP_FE(qsub, NULL)
	PHP_FE(qstat, NULL)
	PHP_FE(qdel, NULL)
	{NULL, NULL, NULL}	/* Must be the last line in drmaa_functions[] */
};
/* }}} */

/* {{{ drmaa_module_entry
 */
zend_module_entry drmaa_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"drmaa",
	drmaa_functions,
	PHP_MINIT(drmaa),
	PHP_MSHUTDOWN(drmaa),
	NULL, /* Thing to do at request start */
	NULL, /* Thing to do at request end */
	PHP_MINFO(drmaa),
#if ZEND_MODULE_API_NO >= 20010901
	"1.2", /* Version number of this extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_DRMAA
ZEND_GET_MODULE(drmaa)
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
	PHP_INI_ENTRY("drmaa.stdlog", ":/tmp", PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("drmaa.errorlog", ":/tmp", PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("drmaa.native",	NULL, PHP_INI_ALL, NULL)
PHP_INI_END()

/* }}} */

/* {{{ php_drmaa_init_globals
 */
static void php_drmaa_init_globals(zend_drmaa_globals *drmaa_globals)
{
	drmaa_globals->global_value = 0;
	drmaa_globals->global_string = NULL;
	if (drmaa_globals->global_init != 1) {
		drmaa_globals->global_init = 0;
	}
}

/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(drmaa)
{
	ZEND_INIT_MODULE_GLOBALS(drmaa,php_drmaa_init_globals, NULL)
	REGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(drmaa)
{
	UNREGISTER_INI_ENTRIES();

	char error[DRMAA_ERROR_STRING_BUFFER];
	char contact[DRMAA_CONTACT_BUFFER];
	int errnum=0;

	if (DRMAA_G(global_init) == 1) {
		errnum = drmaa_exit(error,DRMAA_ERROR_STRING_BUFFER);
		php_printf("DRMAA connection closed");
		DRMAA_G(global_init) = 0;
		if (errnum != DRMAA_ERRNO_SUCCESS) {
			php_printf("Could not shutdown the DRMAA library: %s\n", error);
			/* return FAILURE;*/
		}
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(drmaa)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "drmaa support", "enabled");
	if (DRMAA_G(global_init) == 0) {
		php_info_print_table_row(2, "Drmaa init", "not yet done");
	}
	else {
		php_info_print_table_row(2, "Drmaa init", "done");
	}
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ proto long qstat(string jobid)
	Get the status of a job
* */
PHP_FUNCTION(qstat)
{
	char error[DRMAA_ERROR_STRING_BUFFER];
	int errnum = 0;
	char *jobid = NULL;
	int jobid_len;

	int status = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &jobid, &jobid_len) == FAILURE) {
		RETURN_LONG(0);
	}

	if (DRMAA_G(global_init) == 0) {
		errnum = drmaa_init(NULL, error, DRMAA_ERROR_STRING_BUFFER);
		if (errnum != DRMAA_ERRNO_SUCCESS){
			if (errnum != DRMAA_ERRNO_ALREADY_ACTIVE_SESSION) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "Failed to initialize the DRMAA library: %s", error);
				RETURN_LONG(0);
			}
		}
		DRMAA_G(global_init) = 1;
	}

	errnum = drmaa_job_ps(jobid, &status, error, DRMAA_ERROR_STRING_BUFFER);

	if (errnum != DRMAA_ERRNO_SUCCESS) {
		if (errnum != DRMAA_ERRNO_INVALID_JOB) {
			/* Don't throw an error if the job has not been found because it may be a finished job */
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not get job status for job '%s': %s", jobid, error);
		}
		RETURN_LONG(0);
	}
	else {
		RETURN_LONG(status);
	}
}
/* }}} */

/* {{{ proto bool qdel(string jobid)
	Terminate a job
* */
PHP_FUNCTION(qdel)
{
	char error[DRMAA_ERROR_STRING_BUFFER];
	int errnum = 0;
	char *jobid = NULL;
	int jobid_len;

	int status = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &jobid, &jobid_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (DRMAA_G(global_init) == 0) {
		errnum = drmaa_init(NULL, error, DRMAA_ERROR_STRING_BUFFER);
		if (errnum != DRMAA_ERRNO_SUCCESS){
			if (errnum != DRMAA_ERRNO_ALREADY_ACTIVE_SESSION) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "Failed to initialize the DRMAA library: %s", error);
				RETURN_FALSE;
			}
		}
		DRMAA_G(global_init) = 1;
	}

	errnum = drmaa_control(jobid, DRMAA_CONTROL_TERMINATE, error, DRMAA_ERROR_STRING_BUFFER);

	if (errnum != DRMAA_ERRNO_SUCCESS) {
		if (errnum != DRMAA_ERRNO_INVALID_JOB) {
			/* Don't throw an error if the job has not been found because it may be a finished job */
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not terminate job '%s': %s", jobid, error);
		}
		RETURN_FALSE;
	}
	else {
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ proto string qsub(string command, string job_name, string native)
	Submit the given command to a cluster, with the given job name and native specification if specified.
	If native specification is specified, php.ini drmaa.native setting is ignored.
	Returns the jobid or NULL if the job couldn't be submitted.
* */
PHP_FUNCTION(qsub)
{
	char *command = NULL;
	int command_len;
	char *jobName = NULL;
	int jobName_len;
	char *native = NULL;
	int native_len;
	char error[DRMAA_ERROR_STRING_BUFFER];
	char contact[DRMAA_CONTACT_BUFFER];
	int errnum = 0;
	drmaa_job_template_t *jt = NULL;
	char jobid[DRMAA_JOBNAME_BUFFER];

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ss", &command, &command_len, &jobName, &jobName_len, &native, &native_len) == FAILURE) {
		RETURN_NULL();
	}

	if (DRMAA_G(global_init) == 0) {
		errnum = drmaa_init(NULL, error, DRMAA_ERROR_STRING_BUFFER);
		if (errnum != DRMAA_ERRNO_SUCCESS){
			if (errnum != DRMAA_ERRNO_ALREADY_ACTIVE_SESSION) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "Failed to initialize the DRMAA library: %s", error);
				RETURN_NULL();
			}
		}
		DRMAA_G(global_init)=1;
	}

	errnum = drmaa_allocate_job_template(&jt, error, DRMAA_ERROR_STRING_BUFFER);
	if (errnum != DRMAA_ERRNO_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not create job template: %s", error);
		RETURN_NULL();
	}

	if ((jobName != NULL) && (jobName_len > 0)) {
		errnum = drmaa_set_attribute(jt, DRMAA_JOB_NAME, jobName, error, DRMAA_ERROR_STRING_BUFFER);
		if (errnum != DRMAA_ERRNO_SUCCESS) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not set drmaa job name attribute '%s': %s", jobName, error);
			RETURN_NULL();
		}
	}

	if ((native != NULL) && (native_len > 0)) {
		errnum = drmaa_set_attribute(jt, DRMAA_NATIVE_SPECIFICATION, native, error, DRMAA_ERROR_STRING_BUFFER);
		if (errnum != DRMAA_ERRNO_SUCCESS) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not set drmaa native attribute '%s': %s", native, error);
			RETURN_NULL();
		}
	}
	else if (INI_STR("drmaa.native") != NULL && strlen(INI_STR("drmaa.native")) > 0) {
		errnum = drmaa_set_attribute(jt, DRMAA_NATIVE_SPECIFICATION, INI_STR("drmaa.native"), error, DRMAA_ERROR_STRING_BUFFER);
		if (errnum != DRMAA_ERRNO_SUCCESS) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not set drmaa native attribute '%s': %s", INI_STR("drmaa.native"), error);
			RETURN_NULL();
		}
	}

	errnum = drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, command, error, DRMAA_ERROR_STRING_BUFFER);
	if (errnum != DRMAA_ERRNO_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not set drmaa command attribute '%s': %s", command, error);
		RETURN_NULL();
	}

	errnum = drmaa_set_attribute(jt, DRMAA_OUTPUT_PATH, INI_STR("drmaa.stdlog"), error, DRMAA_ERROR_STRING_BUFFER);
	if (errnum != DRMAA_ERRNO_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not set drmaa stdout attribute '%s': %s", INI_STR("drmaa.stdlog"), error);
		RETURN_NULL();
	}

	errnum = drmaa_set_attribute(jt, DRMAA_ERROR_PATH, INI_STR("drmaa.errorlog"), error, DRMAA_ERROR_STRING_BUFFER);
	if (errnum != DRMAA_ERRNO_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Could not set drmaa stderr attribute '%s': %s", INI_STR("drmaa.errorlog"), error);
		RETURN_NULL();
	}

	errnum = drmaa_run_job(jobid, DRMAA_JOBNAME_BUFFER, jt, error, DRMAA_ERROR_STRING_BUFFER);
	if (errnum != DRMAA_ERRNO_SUCCESS) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Failed to submit job: %s", error);
		RETURN_NULL();
	}

	errnum = drmaa_delete_job_template(jt, error, DRMAA_ERROR_STRING_BUFFER);

	RETURN_STRING(jobid);
}

/* }}} */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
