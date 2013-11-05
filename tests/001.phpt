--TEST--
Check for dav presence
--SKIPIF--
<?php if (!extension_loaded("tnc_dav")) print "skip"; ?>
--FILE--
<?php 
echo "webdav extension is available";
?>
--EXPECT--
webdav extension is available