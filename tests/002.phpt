--TEST--
Check for initialization
--SKIPIF--
<?php
  $host = (false == getenv('WEBDAVHOST')) ? 'stresser' : getenv('WEBDAVHOST');
  $port = (false == getenv('WEBDAVPORT')) ? 8088 : getenv('WEBDAVPORT');

  if(false !== ($fh = fsockopen($host, $port, $errno, $errstr, 2))) {
  	fclose($fh);
  } else {
  	print "skip";
  }
?>
--FILE--
<?php
	$host = (false == getenv('WEBDAVHOST')) ? 'stresser' : getenv('WEBDAVHOST');
  	$port = (false == getenv('WEBDAVPORT')) ? 8088 : getenv('WEBDAVPORT');
	$wdav = new TncWebdav('http://' . $host. ':'.$port);
	var_dump($wdav);
	try {
		$wdav->get('/testy.json');
	} catch (TncWebdavException $e) {
		var_dump($e->getCode());
	}
	var_dump($wdav->put('/testy.json', '{ "test" : "plugin" }'));
	var_dump($wdav->copy('/testy.json', '/testy2.json'));
	var_dump($wdav->get('/testy.json'));
	var_dump($wdav->get('/testy2.json'));
	var_dump($wdav->delete('/testy.json'));
	var_dump($wdav->move('/testy2.json', '/testy.json'));
	var_dump($wdav->get('/testy.json'));
	var_dump($wdav->copy('/testy.json', '/testy2.json'));
	try {
		$wdav->move('/testy.json', '/testy2.json', false);
	} catch (TncWebdavException $e) {
		var_dump($e->getCode());
	}
	var_dump($wdav->move('/testy.json', '/testy2.json', true));
	var_dump($wdav->delete('/testy2.json'));
	try {
		var_dump($wdav->delete('/testy.json'));
	} catch(TncWebdavException $e) {
		var_dump($e->getCode());
	}

?>
--EXPECT--
object(TncWebdav)#1 (0) {
}
int(404)
bool(true)
bool(true)
string(21) "{ "test" : "plugin" }"
string(21) "{ "test" : "plugin" }"
bool(true)
bool(true)
string(21) "{ "test" : "plugin" }"
bool(true)
int(412)
bool(true)
bool(true)
int(404)
