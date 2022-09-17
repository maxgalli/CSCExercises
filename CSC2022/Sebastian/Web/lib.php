<?php

date_default_timezone_set('Europe/Zurich');

if ($PROTECTION == 'localhost') {
	// This code is added here so that this exercise is only visible
	// from the machine that you run it on - to prevent you from hacking
	// machines of your colleagues :-) (Well, at least directly...)
	//
	// (of course it could be also set in iptables or Apache config)
	if ($_SERVER["SERVER_ADDR"] != $_SERVER["REMOTE_ADDR"])
        	die('This exercise is only visible on the localhost itself!<p>' .
                	'Try at <a href="http://localhost/movies">http://localhost/movies</a>');
} else if ($PROTECTION) {
	if ($_SERVER["HTTP_USER_AGENT"] != $PROTECTION)
		die('<h3>Don\'t try hacking in here! Pay attention to the course, instead ;-))</h3>');
}

if ($MULTIDB) {
	$KEY = getArg('key');
	if ($KEY) {
		setcookie('key', $KEY);
		$_user .= substr($KEY, 0, 3);
		$_pwd   = substr($KEY, 3);
		$_database = $_user;
	} else die('Error: please provide the key for your instance, e.g.<br> http://'.$_SERVER['SERVER_NAME'].'/movies?key=1234567890<p>'.
                        'NB: The exercise is <i>NOT</i> about hacking or bruteforcing this key.<br>'.
                        'If you don\'t have it, please subscribe to '.
                        '<a href="https://e-groups.cern.ch/e-groups/EgroupsSubscription.do?egroupName=whitehat-exercise-access">whitehat-exercise-access</a> egroup, '.
                        'and contact Sebastian.Lopienski@cern.ch');



}

// '' (empty - no protection) | 'localhost' | TOKEN (expected as user agent)
$PROTECTION = '';

function getArg($name)
{
	if (array_key_exists($name, $_POST)) return $_POST[$name];
	elseif (array_key_exists($name, $_GET)) return $_GET[$name];
	elseif (array_key_exists($name, $_COOKIE)) return $_COOKIE[$name];
	else return null;
}

?>
