#TncWebdav PHP Extension

The TncWebdav extension is a webdav extension for PHP that uses the [neon library](http://www.webdav.org/neon/ "neon") for webdav operations. It has only been tested with neon version 0.29.6.

##About##

This extension was written by Joseph Lambert joseph at thenetcircle.com [The NetCircle](http://www.thenetcircle.com "The NetCircle Home").

This extension initially started out as few bug fixes for [webdav-1.2 from pureftpd](http://www.pureftpd.org/project/php-webdav "pureftpd"), but turned into more or less a complete rewrite. Instead of returning a resource and passing the resource to different functions, it is rewritten to use a more OO approach, as well as throwing exceptions in most cases instead of returning only false. It also adds support for PROPFIND and OPTIONS methods, as well as adding a getModTime function to return the last modified time of a file. 

However, with all open source software there may be some bugs here and there, so your risk is your own. Also, it is a work in progress.

##Building##

First, you *must install the neon library on the target machine*. See the neon website for information on downloading and installing it. After it is installed, follow these steps:

```php
  phpize
  ./configure --with-tnc-dav
  make
  make test
  sudo make install
```

If the installed neon libraries are not in /usr/local or in /usr, you will need to specify the path to the root directory of the library in the configure step. An example:

```php
  ./configure --with-tnc_dav=/usr/local/neon-0.29.6
```

Next, add the extension to your php.ini file:

```php
  extension=tnc_dav.so
```

##Note for Windows users##

Good luck.

##Usage and supported features##

The code currently allows the following operations in webdav:
* GET
* PUT
* DELETE
* MKCOL
* COPY
* MOVE

LOCK, UNLOCK and PROPPATCH may be added, but there are no plans for this yet.

Also to be added is handling for HTTPS as well as "other" functions for a more complete implementation.

##Usage##

The following is a usage example:

```php
    
    $connection = new TncWebdav("http://localhost:8088", "username", "password");
    
    // Returns true, or throws a TncWebdavException
    // Put a file to the path /my_image.jpg
    $connection->put("/my_image.jpg", file_get_contents("./test_image.jpg"));
    
    // Get last modified time
    echo $connection->lastModTime('/my_image.jpg'); // Tue Sep  4 14:07:01 2012
    
    // NOTE: you need the trailing slash (mkcol == mkdir)
    $connection->mkcol("/testing/");
    
    // Move a file from one location to another
    $connection->move("/my_image.jpg", "/testing/test_image2.jpg");
    
    // Copy from /testing/test_image2.jpg to /test_image3.jpg
    $connection->copy("/testing/test_image2.jpg", "/test_image3.jpg");
    
    // get file /testing/test_image2.jpg (as a string)
    file_put_contents("./my_new_image.jpg", $connection->get("/testing/test_image2.jpg"));
    
    // delete file /testing/test_image2.jpg
    $connection->delete("/testing/test_image2.jpg");
    
    // delete file /test_image3.jpg
    $connection->delete("/test_image3.jpg");
    
    // delete directory (must be empty) - REMEMBER THE TRAILING SLAAAASH
    $connection->delete("/testing/");
    
    try {
      $connection->delete("/test_image3.jpg");
    } catch (TncWebdavException $e) {
      echo $e->getMessage(); // "404 Not Found"
      echo $e->getCode(); // 404
    }

```

All functions will throw a TncWebdavException if they don't have a 2xx HTTP status code for the operation. The exception message will be the text returned from the DAV server and the error code will be the HTTP status code.

###PROPFIND###

The propfind method will return an XML-formatted string of properties for a file. However, it seems the SimpleXML and XMLReader classes in PHP cannot properly parse it. It is formatted according to the [WebDAV RFC](http://www.webdav.org/specs/rfc2518.html#METHOD_PROPFIND "PROPFIND").

###OPTIONS###

Options will return an array of booleans that correspond to the following class constants:

* TncWebdav::CAP_DAV_CLASS1 - Class 1 WebDAV (RFC 2518)
* TncWebdav::CAP_DAV_CLASS2 - Class 2 WebDAV (RFC 2518) 
* TncWebdav::CAP_DAV_CLASS3 - Class 3 WebDAV (RFC 4918) 
* TncWebdav::CAP_MODDAV_EXEC - mod_dav "executable" property 
* TncWebdav::CAP_DAV_ACL - WebDAV ACL (RFC 3744) 
* TncWebdav::CAP_VER_CONTROL - DeltaV version-control 
* TncWebdav::CAP_CO_IN_PLACE - DeltaV checkout-in-place 
* TncWebdav::CAP_VER_HISTORY - DeltaV version-history 
* TncWebdav::CAP_WORKSPACE - DeltaV workspace 
* TncWebdav::CAP_UPDATE - DeltaV update 
* TncWebdav::CAP_LABEL - DeltaV label 
* TncWebdav::CAP_WORK_RESOURCE - DeltaV working-resouce 
* TncWebdav::CAP_MERGE - DeltaV merge 
* TncWebdav::CAP_BASELINE - DeltaV baseline 
* TncWebdav::CAP_ACTIVITY - DeltaV activity 
* TncWebdav::CAP_VC_COLLECTION - DeltaV version-controlled-collection 

For example:

```php
  
  $options = $c->options('/test_image.jpg');
  
  echo $options[TncWebdav::CAP_DAV_CLASS1]; // bool(true) if it implements RFC 2518

```

These map to macros in src/ne\_basic.h in the neon library. If it returns true, the WebDAV resource has this capability.

