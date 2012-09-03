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

#include "php_tnc_dav.h"

zend_class_entry *tnc_dav_ce;
zend_class_entry *tnc_dav_exception_ce;

zend_object_handlers tnc_dav_default_handlers;

/* TODO: define some constants for error codes */

/* TODO: need static keyword? */
static function_entry tnc_dav_methods[] = {
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
  zend_hash_copy(intern->std.properties,                         
     &class_type->default_properties,                            
     (copy_ctor_func_t) zval_add_ref,                            
     (void *) &tmp,                                              
     sizeof(zval *));                                            
                                                                 
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
	if(NULL == (sess = ne_session_create(uri.scheme, uri.host, uri.port)))
	{
		zend_throw_exception(tnc_dav_exception_ce, "Unable to open a new DAV session", 0 TSRMLS_CC);
	}
	
	ne_set_read_timeout(sess, (int) timeout);
	object->sess = sess;
	if(NULL != user && NULL != pass)
	{
		ne_set_server_auth(sess, cb_dav_auth, object);
	}
	
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
	
	ne_request *req;
	tnc_dav_t *object;
	
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &uri, &uri_len))
	{
		// throw exception?
		return;
	}
	
	GET_DAV(getThis());
	// full uri? look at get_full_uri
	full_uri = make_full_uri(object, uri, uri_len);
	req = ne_request_create(object->sess, "GET", uri);
	RETVAL_EMPTY_STRING();
	ne_add_response_body_reader(req, ne_accept_2xx, cb_dav_reader, return_value);
	ret = ne_request_dispatch(req);
	ne_request_destroy(req);
	
	if(NE_OK != ret || 2 != ne_get_status(req)->klass)
	{
		char *err_msg;
		err_msg = get_error_msg(object->sess);
		zend_throw_exception(tnc_dav_exception_ce, err_msg, 0 TSRMLS_CC);
		efree(err_msg);
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
	
	full_uri = make_full_uri(object, uri, uri_len);
	req = ne_request_create(object->sess, "PUT", uri);
	ne_set_request_body_buffer(req, data, data_len);
	ret = ne_request_dispatch(req);
	ne_request_destroy(req);
	
	if(NE_OK != ret || 2 != ne_get_status(req)->klass)
	{
		char *err_msg;
		err_msg = get_error_msg(object->sess);
		zend_throw_exception(tnc_dav_exception_ce, err_msg, 0 TSRMLS_CC);
		efree(err_msg);
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
	
	full_uri = make_full_uri(object, uri, uri_len);
	req = ne_request_create(object->sess, "DELETE", uri);
	ret = ne_simple_request(object->sess, req);
	
	if(NE_OK != ret || 2 != ne_get_status(req)->klass)
	{
		char *err_msg;
		err_msg = get_error_msg(object->sess);
		zend_throw_exception(tnc_dav_exception_ce, err_msg, 0 TSRMLS_CC);
		efree(err_msg);
	}
	
	RETURN_TRUE;
}


/**
 * TODO: does not work, always gives back an error
 */
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
		zend_throw_exception(tnc_dav_exception_ce, err_msg, 0 TSRMLS_CC);
		efree(err_msg);
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
		zend_throw_exception(tnc_dav_exception_ce, err_msg, 0 TSRMLS_CC);
		efree(err_msg);
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
	
	full_uri = make_full_uri(object, uri, uri_len);
	req = ne_request_create(object->sess, "MKCOL", full_uri);
    ret = ne_simple_request(object->sess, req);
	
	if (ret != NE_OK || ne_get_status(req)->klass != 2) 
	{
		char *err_msg;
		err_msg = get_error_msg(object->sess);
		zend_throw_exception(tnc_dav_exception_ce, err_msg, 0 TSRMLS_CC);
		efree(err_msg);
	}
	
	RETURN_TRUE;
}