#TncWebdav PHP Extension

The TncWebdav extension is a webdav extension for PHP that uses the [neon library](http://www.webdav.org/neon/ "neon") for webdav operations.

##Building##

First, you must install the neon library on the target machine. See the neon source code for information on how to install it. After it is installed, follow these steps:

<pre><code>
  phpize
  ./configure --with-tnc-dav
  make
  make test
  sudo make install
</code></pre>

If the installed neon libraries are not in /usr/local or in /usr, you will need to specify the path to the root directory of the library in the configure step. An example:

<pre><code>
  ./configure --with-tnc\_dav=/usr/local/neon-0.29.6
</code></pre>

Next, add the extension to your php.ini file:

<pre><code>
  extension=tnc\_dav.so
</code></pre>

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

In the future, PROPFIND, PROPPATCH and OPTIONS will be added. LOCK and UNLOCK may be added, but there are no plans for this yet.

###Usage###

The following is a usage example:

<pre><code>
  <?php
    $connection = new TncWebdav("http://localhost:8088", "username", "password");
    
    // Returns true, or throws a TncWebdavException
    $connection->put("/my_image.jpg", file_get_contents("./test_image.jpg"));
    
    // NOTE: you need the trailing slash (mkcol == mkdir)
    $connection->mkcol("/testing/");
    
    $connection->move("/test_image.jpg", "/testing/test_image2.jpg");
    $connection->copy("/testing/test_image2.jpg", "/test_image3.jpg");
    
    file_put_contents("./my_new_image.jpg", $connection->get("/testing/test_image2.jpg"));
    
    $connection->delete("/testing/test_image2.jpg");
    $connection->delete("/test_image3.jpg");
  ?>
</code></pre>

All functions will throw a TncWebdavException if they don't have a 2xx HTTP status code for the operation.