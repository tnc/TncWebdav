/*
	Copyright 2012 The NetCircle <joseph@thenetcircle.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <php.h>
#include "php_ini.h"
#include "ext/standard/info.h"
#include <ne_socket.h>
#include <ne_session.h>
#include <ne_utils.h>
#include <ne_auth.h>
#include <ne_basic.h>
#include <ne_207.h>
#include <zend_exceptions.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "php_tnc_dav.h"

zend_class_entry *tnc_dav_ce;
zend_class_entry *tnc_dav_exception_ce;

zend_object_handlers tnc_dav_default_handlers;

/* TODO: need static keyword? */
static zend_function_entry tnc_dav_methods[] = {
	PHP_ME(TncWebdav, __construct, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(TncWebdav, getHost, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, getUser, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, getPassword, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, get, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, put, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, delete, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, copy, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, move, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, mkcol, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, propfind, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, options, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, getModTime, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, close, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, setReadTimeout, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, setConnectTimeout, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(TncWebdav, getStatus, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

zend_module_entry tnc_dav_module_entry = {
	STANDARD_MODULE_HEADER,
	PHP_TNC_DAV_EXTNAME,
	NULL,
	PHP_MINIT(tnc_dav),
	NULL,
	NULL,
	NULL,
	NULL,
	PHP_TNC_DAV_VERSION,
	STANDARD_MODULE_PROPERTIES
};

/* {{{ */
ZEND_GET_MODULE(tnc_dav)
/* }}} */


PHP_MINIT_FUNCTION(tnc_dav)
{
	zend_class_entry ce, e;
	
	INIT_CLASS_ENTRY(ce, "TncWebdav", tnc_dav_methods);
	ce.create_object = create_new_tnc_dav;
	tnc_dav_ce = zend_register_internal_class(&ce TSRMLS_CC);
	
	INIT_CLASS_ENTRY(e, "TncWebdavException", NULL);
	
	#if ZEND_MODULE_API_NO >= 20060613
	  tnc_dav_exception_ce = zend_register_internal_class_ex(&e, (zend_class_entry*)zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
	#else
	  tnc_dav_exception_ce = zend_register_internal_class_ex(&e, (zend_class_entry*)zend_exception_get_default(), NULL TSRMLS_CC);
	#endif /* ZEND_MODULE_API_NO >= 20060613 */
	
	memcpy(&tnc_dav_default_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	tnc_dav_default_handlers.clone_obj = NULL;
	
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_DAV_CLASS1", strlen("CAP_DAV_CLASS1"), 0 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_DAV_CLASS2", strlen("CAP_DAV_CLASS2"), 1 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_DAV_CLASS3", strlen("CAP_DAV_CLASS3"), 2 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_MODDAV_EXEC", strlen("CAP_MODDAV_EXEC"), 3 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_DAV_ACL", strlen("CAP_DAV_ACL"), 4 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_VER_CONTROL", strlen("CAP_VER_CONTROL"), 5 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_CO_IN_PLACE", strlen("CAP_CO_IN_PLACE"), 6 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_VER_HISTORY", strlen("CAP_VER_HISTORY"), 7 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_WORKSPACE", strlen("CAP_WORKSPACE"), 8 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_UPDATE", strlen("CAP_UPDATE"), 9 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_LABEL", strlen("CAP_LABEL"), 10 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_WORK_RESOURCE", strlen("CAP_WORK_RESOURCE"), 11 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_MERGE", strlen("CAP_MERGE"), 12 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_BASELINE", strlen("CAP_BASELINE"), 13 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_ACTIVITY", strlen("CAP_ACTIVITY"), 14 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "CAP_VC_COLLECTION", strlen("CAP_VC_COLLECTION"), 15 TSRMLS_CC);
	
	zend_declare_class_constant_long(tnc_dav_ce, "STATUS_CONNECTING", strlen("STATUS_CONNECTING"), 0 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "STATUS_CONNECTED", strlen("STATUS_CONNECTED"), 1 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "STATUS_DISCONNECTED", strlen("STATUS_DISCONNECTED"), 2 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "STATUS_LOOKUP", strlen("STATUS_LOOKUP"), 3 TSRMLS_CC);
	zend_declare_class_constant_long(tnc_dav_ce, "STATUS_UNKNOWN", strlen("STATUS_UNKNOWN"), 4 TSRMLS_CC);
	
	/* status codes - should we map these to some specific constants?
		200 OK
		201 Created
		204 No Content
		403 Forbidden
		404 Not Found
		405 Method Not Allowed
		409 Conflict
		412 Precondition Failed
		423 Locked
		415 Unsupported Media Type
		500 Internal Server Error
		502 Bad Gateway
		507 Insufficient Storage
	*/
	
	return SUCCESS;
}

static zend_object_value create_new_tnc_dav(zend_class_entry *class_type TSRMLS_DC)
{
	//php_tnc_dav_obj_new(tnc_dav_t);
  zend_object_value retval;                             
  tnc_dav_t *intern;                                  
  zval *tmp;                                            
                                                        
  intern = (tnc_dav_t*)emalloc(sizeof(tnc_dav_t));               
  memset(intern, 0, sizeof(tnc_dav_t));                          
                                                                 
  zend_object_std_init(&intern->std, class_type TSRMLS_CC);      
  /*zend_hash_copy(intern->std.properties,                         
     &class_type->default_properties,                            
     (copy_ctor_func_t) zval_add_ref,                            
     (void *) &tmp,                                              
     sizeof(zval *));*/

  init_properties(intern);                                            
                                                                 
  retval.handle = zend_objects_store_put(intern,                 
     (zend_objects_store_dtor_t) zend_objects_destroy_object,    
     php_tnc_dav_t_free, NULL TSRMLS_CC);                    
  retval.handlers = &tnc_dav_default_handlers;       

  return retval;
}

void php_tnc_dav_t_free(void *object TSRMLS_DC)
{
	tnc_dav_t *obj = (tnc_dav_t*)object;
	if(obj->url)
	{
		efree(obj->url);
	}
	if(obj->user)
	{
		efree(obj->user);
	}
	if(obj->pass)
	{
		efree(obj->pass);
	}
	if(obj->sess)
	{
		ne_session_destroy(obj->sess);
	}
	efree(obj);
}

static char *get_error_msg(ne_session *sess) 
{
	char buffer[255];
    char *errmsg;
    int errlen;
	//printf("%s", ne_get_error(sess));
	
	errlen = sprintf(buffer, "%s", ne_get_error(sess));
	errmsg = (char*)emalloc(errlen + 1);
	
	memcpy(errmsg, buffer, errlen);
	errmsg[errlen] = '\0';
	
	return errmsg;
}

static char *make_full_uri(tnc_dav_t *object, const char *path, int path_len)
{
	char *full_uri;
	full_uri = (char *)malloc(object->url_len + path_len + 1);
	memcpy(full_uri, object->url, object->url_len);
	memcpy(full_uri + object->url_len, path, path_len);
	full_uri[object->url_len + path_len] = '\0';
	return full_uri;
}


static int cb_dav_auth(void *userdata, const char *realm, int attempts,
                       char *username, char *password) {
    const tnc_dav_t *dav_session = (const tnc_dav_t *) userdata;
    
    strlcpy(username, dav_session->user, NE_ABUFSIZ);
    strlcpy(password, dav_session->pass, NE_ABUFSIZ);
    
    return attempts;
}

/* no null-teminated string stuff here, but how do we do it? this is called in a loop */
static int cb_dav_reader(void *userdata, const char *buf, size_t len) {
    zval * const return_value = (zval * const) userdata;
    size_t old_len, full_len;

    if (len <= (size_t) 0U) {
        return 0;
    }
    old_len = Z_STRLEN_P(return_value);
    full_len = old_len + len;
    if (full_len < old_len || full_len < len) {
        return -1;
    }

    Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), full_len);
    Z_STRLEN_P(return_value) = full_len;
    memcpy(Z_STRVAL_P(return_value) + old_len, buf, len);

    return 0;
}

static void status_callback(void *userdata, ne_session_status status,
                      const ne_session_status_info *info)
{
	
	tnc_dav_t *object = (tnc_dav_t *)userdata;
    char scratch[512];
	int len = 0;
	
    switch (status) {
    case ne_status_lookup:
		len = sprintf(scratch, "lookup(%s)", (char *)info->lu.hostname);
		object->status_buffer = erealloc(object->status_buffer, len + 1);
		memcpy(object->status_buffer, &scratch, len);
		object->status_buffer[len] = '\0';
		object->status_code = 3;
        break;
    case ne_status_connecting:
		len = sprintf(scratch, "connecting(%s)", (char *)info->lu.hostname);
		object->status_buffer = erealloc(object->status_buffer, len + 1);
		memcpy(object->status_buffer, &scratch, len);
		object->status_buffer[len] = '\0';
		object->status_code = 0;
        break;
    case ne_status_sending:
		break;
    case ne_status_recving:
		break;
	/*	if(ne_status_sending) {
			len = sprintf(scratch, "sending %d of %d", info->sr.progress, info->sr.total);
		} else {
			len = sprintf(scratch, "received %d of %d", info->sr.progress, info->sr.total);
		}
		object->status_buffer = erealloc(object->status_buffer, len + 1);
		memcpy(object->status_buffer, &scratch, len);
		object->status_buffer[len] = '\0';
        break;*/
    case ne_status_disconnected:
		object->status_buffer = erealloc(object->status_buffer, 13);
		memcpy(object->status_buffer, "disconnected", 12);
		object->status_buffer[12] = '\0';
		object->status_code = 2;
		break;
    case ne_status_connected:
		len = sprintf(scratch, "connected(%s)", (char *)info->cd.hostname);
		object->status_buffer = erealloc(object->status_buffer, len + 1);
		memcpy(object->status_buffer, &scratch, len);
		object->status_buffer[len] = '\0';
		object->status_code = 1;
        break;
    default:
		//object->status_buffer = erealloc(object->status_buffer, 5);
		//memcpy(object->status_buffer, "test", 4);
		//object->status_buffer[4] = '\0';
        break;
    }
}

int tnc_dav_accept_2xx(void *userdata, ne_request *req, const ne_status *st)
{
	return (st->klass == 2);
}

PHP_METHOD(TncWebdav, __construct)
{
	char *host;
	char *user;
	char *pass;
	int host_len, user_len, pass_len;
	long timeout = 5L;
	
	ne_session *sess;
	ne_uri uri;
	
	memset(&uri, 0, sizeof(uri));
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ss", &host, &host_len, &user, &user_len, &pass, &pass_len) == FAILURE)
	{
		return;
	}
	
	tnc_dav_t *object;
	
	object = (tnc_dav_t*)zend_object_store_get_object(getThis() TSRMLS_CC);
	object->url = estrdup(host);
	object->url_len = host_len;
	
	object->sess = sess;
	
	if (ne_uri_parse(host, &uri) != 0) {
		zend_throw_exception(tnc_dav_exception_ce, "Invalid base URL", 0 TSRMLS_CC);
    }
	// TODO: timeout
	
	if(ZEND_NUM_ARGS() TSRMLS_CC > 1)
	{
		object->user = estrdup(user);
		object->user_len = user_len;
	}
	
	if(ZEND_NUM_ARGS() TSRMLS_CC > 2)
	{
		object->pass = estrdup(pass);
		object->pass_len = pass_len;
	}
	
	object->status_buffer = (char *)emalloc(2);
	memcpy(object->status_buffer, "0", 1);
	object->status_buffer[1] = '\0';
	
	// set status unknown
	object->status_code = 4;
	
	// make connection then
	if(NULL == uri.scheme) 
	{
		uri.scheme = "http";
	}
	if(0 == uri.port)
	{
		uri.port = ne_uri_defaultport(uri.scheme);
	}
	if(ne_sock_init())
	{
		zend_throw_exception(tnc_dav_exception_ce, "Unable to initialize socket libraries", 0 TSRMLS_CC);
	}
	if(NULL == (object->sess = ne_session_create(uri.scheme, uri.host, uri.port)))
	{
		zend_throw_exception(tnc_dav_exception_ce, "Unable to open a new DAV session", 0 TSRMLS_CC);
	}
	
	ne_set_notifier(object->sess, status_callback, object);
	
	ne_set_read_timeout(object->sess, (int) timeout);
	
	
	if(NULL != user && NULL != pass)
	{
		ne_set_server_auth(object->sess, cb_dav_auth, object);
	}
	
}

/**
 * If explicitly called, we don't want to destroy the session, as it
 * will try to destroy a non-existent resource in the destruct method
 */
PHP_METHOD(TncWebdav, close)
{
	tnc_dav_t *object;
	GET_DAV(getThis());
	
	ne_close_connection(object->sess);	
	
	RETURN_TRUE;
}

PHP_METHOD(TncWebdav, getHost)
{
	tnc_dav_t *object;
	
	GET_DAV(getThis());
	RETURN_STRING(object->url, 1);
}

PHP_METHOD(TncWebdav, getUser)
{
	tnc_dav_t *object;
	
	GET_DAV(getThis());
	if(NULL != object->user)
	{
		RETURN_STRING(object->user, 1);
	}
	
	RETURN_NULL();
}

PHP_METHOD(TncWebdav, getPassword)
{
	tnc_dav_t *object;
	
	GET_DAV(getThis());
	if(NULL != object->pass)
	{
		RETURN_STRING(object->pass, 1);
	}
	
	RETURN_NULL();
}

PHP_METHOD(TncWebdav, get)
{
	char *uri;
	char *full_uri;
	int uri_len;
	int ret;
	
	char *tmp_return;
	
	ne_request *req;
	tnc_dav_t *object;
	
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &uri, &uri_len))
	{
		// throw exception?
		return;
	}
	
	GET_DAV(getThis());
	// full uri? look at get_full_uri
	//full_uri = make_full_uri(object, uri, uri_len);
	req = ne_request_create(object->sess, "GET", uri);
	RETVAL_EMPTY_STRING();
	ne_add_response_body_reader(req, tnc_dav_accept_2xx, cb_dav_reader, return_value);
	ret = ne_request_dispatch(req);
	ne_request_destroy(req);
	
	int old_len = Z_STRLEN_P(return_value);
	tmp_return = (char *)emalloc(old_len + 1);
	memcpy(tmp_return, Z_STRVAL_P(return_value), old_len);
	tmp_return[old_len] = '\0';
	
	Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), old_len + 1);
	memcpy(Z_STRVAL_P(return_value), tmp_return, old_len + 1);
	
 	efree(tmp_return);
	
	if(NE_OK != ret || 2 != ne_get_status(req)->klass)
	{
		char *err_msg;
		err_msg = get_error_msg(object->sess);
		// get error code
		char *code = (char *)emalloc(3);
		memcpy(code, err_msg, 3);
		zend_throw_exception(tnc_dav_exception_ce, err_msg, atoi(code) TSRMLS_CC);
		efree(err_msg);
		efree(code);
	}
	
}

PHP_METHOD(TncWebdav, put)
{
	char *uri, *data;
	int uri_len, data_len;
	char *full_uri;
	int ret;
	
	ne_request *req;
	tnc_dav_t *object;
	
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &uri, &uri_len, &data, &data_len))
	{
		return;
	}
	
	GET_DAV(getThis());
	
	//full_uri = make_full_uri(object, uri, uri_len);
	req = ne_request_create(object->sess, "PUT", uri);
	ne_set_request_body_buffer(req, data, data_len);
	ret = ne_request_dispatch(req);
	ne_request_destroy(req);
	
	if(NE_OK != ret || 2 != ne_get_status(req)->klass)
	{
		char *err_msg;
		err_msg = get_error_msg(object->sess);
		// get error code
		char *code = (char *)emalloc(3);
		memcpy(code, err_msg, 3);
		zend_throw_exception(tnc_dav_exception_ce, err_msg, atoi(code) TSRMLS_CC);
		efree(err_msg);
		efree(code);
	}
	
	RETURN_TRUE;
}

PHP_METHOD(TncWebdav, delete)
{
	char *uri;
	int uri_len;
	char *full_uri;
	int ret;
	
	ne_request *req;
	tnc_dav_t *object;
	
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &uri, &uri_len))
	{
		return;
	}
	
	GET_DAV(getThis());
	
	//full_uri = make_full_uri(object, uri, uri_len);
	//req = ne_request_create(object->sess, "DELETE", uri);
	//ret = ne_simple_request(object->sess, req);
	ret = ne_delete(object->sess, uri);
	
	if(NE_OK != ret)
	{
		char *err_msg;
		err_msg = get_error_msg(object->sess);
		// get error code
		char *code = (char *)emalloc(3);
		memcpy(code, err_msg, 3);
		zend_throw_exception(tnc_dav_exception_ce, err_msg, atoi(code) TSRMLS_CC);
		efree(err_msg);
		efree(code);
	}
	
	RETURN_TRUE;
}


PHP_METHOD(TncWebdav, copy)
{
	char *source_uri, *dest_uri;
	int source_uri_len, dest_uri_len;
	
	char *full_uri_source, *full_uri_dest;
	int ret;
	zend_bool overwrite;
	
	ne_request *req;
	tnc_dav_t *object;
	
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|b", &source_uri, &source_uri_len, &dest_uri, &dest_uri_len, &overwrite))
	{
		return;
	}
	
	GET_DAV(getThis());
		
	// TODO: overwrite parameter and Depth
	// NE_DEPTH_INFINITE or NE_DEPTH_ZERO
	
	if(3 > ZEND_NUM_ARGS())
	{
		overwrite = 1;
	}
	
	ret = ne_copy(object->sess, (int)overwrite, NE_DEPTH_INFINITE, source_uri, dest_uri);
	//efree(source_uri);
	//efree(dest_uri);
	
	if(NE_OK != ret)
	{
		char *err_msg;
		err_msg = get_error_msg(object->sess);
		// get error code
		char *code = (char *)emalloc(3);
		memcpy(code, err_msg, 3);
		zend_throw_exception(tnc_dav_exception_ce, err_msg, atoi(code) TSRMLS_CC);
		efree(err_msg);
		efree(code);
	}
	
	RETURN_TRUE;
}

PHP_METHOD(TncWebdav, move)
{
	char *source_uri, *dest_uri;
	int source_uri_len, dest_uri_len;
	
	int ret;
	
	zend_bool overwrite;
	tnc_dav_t *object;
	
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|b", &source_uri, &source_uri_len, &dest_uri, &dest_uri_len, &overwrite))
	{
		return;
	}
	
	if(3 > ZEND_NUM_ARGS())
	{
		overwrite = 1;
	}
	
	GET_DAV(getThis());
	
	ret = ne_move(object->sess, (int)overwrite, source_uri, dest_uri);
	
	if(NE_OK != ret)
	{
		char *err_msg;
		err_msg = get_error_msg(object->sess);
		// get error code
		char *code = (char *)emalloc(3);
		memcpy(code, err_msg, 3);
		zend_throw_exception(tnc_dav_exception_ce, err_msg, atoi(code) TSRMLS_CC);
		efree(err_msg);
		efree(code);
	}
	
	RETURN_TRUE;
}

PHP_METHOD(TncWebdav, mkcol)
{
	char *uri, *full_uri;
	int uri_len;
	
	int ret;
	
	ne_request *req;
	tnc_dav_t *object;
	
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &uri, &uri_len))
	{
		return;
	}
	
	GET_DAV(getThis());
	
	//full_uri = make_full_uri(object, uri, uri_len);
	//req = ne_request_create(object->sess, "MKCOL", full_uri);
    //ret = ne_simple_request(object->sess, req);
	ret = ne_mkcol(object->sess, uri);

	if (ret != NE_OK) 
	{
		char *err_msg;
		err_msg = get_error_msg(object->sess);
		// get error code
		char *code = (char *)emalloc(3);
		memcpy(code, err_msg, 3);
		zend_throw_exception(tnc_dav_exception_ce, err_msg, atoi(code) TSRMLS_CC);
		efree(err_msg);
		efree(code);
	}
	
	RETURN_TRUE;
}

PHP_METHOD(TncWebdav, propfind)
{
	char *uri, *full_uri;
	int uri_len;
	
	char *tmp_return;
	
	int ret;
	
	ne_request *req;
	tnc_dav_t *object;
	
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &uri, &uri_len))
	{
		return;
	}
	
	GET_DAV(getThis());
	
	full_uri = make_full_uri(object, uri, uri_len);
	req = ne_request_create(object->sess, "PROPFIND", uri);
	RETVAL_EMPTY_STRING();
	ne_add_response_body_reader(req, tnc_dav_accept_2xx, cb_dav_reader, return_value);
	ret = ne_request_dispatch(req);
	ne_request_destroy(req);
	
	int old_len = Z_STRLEN_P(return_value);
	tmp_return = (char *)emalloc(old_len + 1);
	memcpy(tmp_return, Z_STRVAL_P(return_value), old_len);
	tmp_return[old_len] = '\0';
	
	Z_STRVAL_P(return_value) = erealloc(Z_STRVAL_P(return_value), old_len + 1);
	memcpy(Z_STRVAL_P(return_value), tmp_return, old_len + 1);
	
 	efree(tmp_return);

	if(NE_OK != ret || 2 != ne_get_status(req)->klass)
	{
		char *err_msg;
		err_msg = get_error_msg(object->sess);
		// get error code
		char *code = (char *)emalloc(3);
		memcpy(code, err_msg, 3);
		zend_throw_exception(tnc_dav_exception_ce, err_msg, atoi(code) TSRMLS_CC);
		efree(err_msg);
		efree(code);
	}
}

PHP_METHOD(TncWebdav, options)
{
	char *uri;
	int uri_len;
	int ret;
	tnc_dav_t *object;
	
	unsigned int capabilities;
	
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &uri, &uri_len))
	{
		return;
	}
	
	GET_DAV(getThis());
	
	ret = ne_options2(object->sess, uri, &capabilities);
	
	array_init(return_value);
	add_index_bool(return_value, 0, capabilities & NE_CAP_DAV_CLASS1);
	add_index_bool(return_value, 1, capabilities & NE_CAP_DAV_CLASS2);
	add_index_bool(return_value, 2, capabilities & NE_CAP_DAV_CLASS3);
	add_index_bool(return_value, 3, capabilities & NE_CAP_MODDAV_EXEC);
	add_index_bool(return_value, 4, capabilities & NE_CAP_DAV_ACL);
	add_index_bool(return_value, 5, capabilities & NE_CAP_VER_CONTROL);
	add_index_bool(return_value, 6, capabilities & NE_CAP_CO_IN_PLACE);
	add_index_bool(return_value, 7, capabilities & NE_CAP_VER_HISTORY);
	add_index_bool(return_value, 8, capabilities & NE_CAP_WORKSPACE);
	add_index_bool(return_value, 9, capabilities & NE_CAP_UPDATE);
	add_index_bool(return_value, 10, capabilities & NE_CAP_LABEL);
	add_index_bool(return_value, 11, capabilities & NE_CAP_WORK_RESOURCE);
	add_index_bool(return_value, 12, capabilities & NE_CAP_MERGE);
	add_index_bool(return_value, 13, capabilities & NE_CAP_BASELINE);
	add_index_bool(return_value, 14, capabilities & NE_CAP_ACTIVITY);
	add_index_bool(return_value, 15, capabilities & NE_CAP_VC_COLLECTION);
	
	if(NE_OK != ret)
	{
		char *err_msg;
		err_msg = get_error_msg(object->sess);
		// get error code
		char *code = (char *)emalloc(3);
		memcpy(code, err_msg, 3);
		zend_throw_exception(tnc_dav_exception_ce, err_msg, atoi(code) TSRMLS_CC);
		efree(err_msg);
		efree(code);
	}
}

PHP_METHOD(TncWebdav, getModTime)
{
	char *uri;
	int uri_len;
	
	char *ret_modtime;
	int ret;
	tnc_dav_t *object;
	
	time_t modtime;
	
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &uri, &uri_len))
	{
		return;
	}
	
	GET_DAV(getThis());
	
	ret = ne_getmodtime(object->sess, uri, &modtime);

	if(NE_OK != ret)
	{
		char *err_msg;
		err_msg = get_error_msg(object->sess);
		// get error code
		char *code = (char *)emalloc(3);
		memcpy(code, err_msg, 3);
		zend_throw_exception(tnc_dav_exception_ce, err_msg, atoi(code) TSRMLS_CC);
		efree(err_msg);
		efree(code);
	}
	
	// get rid of the stupid newline character
	if(-1 != modtime)
	{
		int len = strlen(ctime(&modtime));
		ret_modtime = (char *)emalloc(len - 1);
		memcpy(ret_modtime, ctime(&modtime), len);
		ret_modtime[len - 1] = '\0';
	
		RETURN_STRING(ret_modtime, 1);
		efree(ret_modtime);
	}
	
	RETURN_FALSE;
}

PHP_METHOD(TncWebdav, setReadTimeout)
{
	long timeout;
	
	tnc_dav_t *object;
	
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &timeout))
	{
		return;
	}
	
	GET_DAV(getThis());
	
	ne_set_read_timeout(object->sess, (int)timeout);
	
	RETURN_TRUE;
}

PHP_METHOD(TncWebdav, setConnectTimeout)
{
	long timeout;
	
	tnc_dav_t *object;
	
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &timeout))
	{
		return;
	}
	
	GET_DAV(getThis());
	
	ne_set_connect_timeout(object->sess, (int)timeout);
	
	RETURN_TRUE;
}

PHP_METHOD(TncWebdav, getStatus)
{
	tnc_dav_t* object;
	
	GET_DAV(getThis());
	
	array_init(return_value);
	add_assoc_long(return_value, "code", object->status_code);
	add_assoc_string(return_value, "message", object->status_buffer, 1);
}