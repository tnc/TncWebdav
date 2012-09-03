#ifndef PHP_TNC_DAV_H
#define PHP_TNC_DAV_H

#define PHP_TNC_DAV_EXTNAME "tnc_dav"
#define PHP_TNC_DAV_VERSION "0.1.0"

typedef struct _tnc_dav {
	zend_object std;
	char *url;
	int url_len;
	char *user;
	int user_len;
	char *pass;
	int pass_len;
	ne_session *sess;
} tnc_dav_t;

#define GET_DAV(obj)  \
  object = (tnc_dav_t*)zend_object_store_get_object((obj) TSRMLS_CC);
  

static zend_object_value create_new_tnc_dav(zend_class_entry *class_type TSRMLS_DC);
void php_tnc_dav_t_free(void *object TSRMLS_DC);
static int cb_dav_auth(void *userdata, const char *realm, int attempts, char *username, char *password);
static int cb_dav_reader(void *userdata, const char *buf, size_t len);
static char *get_error_msg(ne_session *sess);
static char *make_full_uri(tnc_dav_t *object, const char *path, int path_len);

PHP_MINIT_FUNCTION(tnc_dav);

PHP_METHOD(TncWebdav, __construct);
PHP_METHOD(TncWebdav, getHost);
PHP_METHOD(TncWebdav, getUser);
PHP_METHOD(TncWebdav, getPassword);
PHP_METHOD(TncWebdav, get);
PHP_METHOD(TncWebdav, put);
PHP_METHOD(TncWebdav, delete);
PHP_METHOD(TncWebdav, copy);
PHP_METHOD(TncWebdav, move);
PHP_METHOD(TncWebdav, mkcol);

#endif