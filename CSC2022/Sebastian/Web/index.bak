<?php
header("X-XSS-Protection: 0");

require("lib.php");

$page = getArg("p");
if ($page == "") $page = 'main';

require("db.php");
initDB('localhost', 'cscsecex', 'WbjRl.18aswo', 'cscsecex');

?>

<html>
	<head>
		<title>A(nother) great, secure movie database</title>
	</head>
<body>

<h2>A(nother) great, secure movie database</h2>
<b>
<a href="index.php?p=main">home</a>&nbsp;&nbsp;&nbsp;
<a href="index.php?p=search&q=all">all movies</a>&nbsp;&nbsp;&nbsp;
<a href="index.php?p=search">search</a>&nbsp;&nbsp;&nbsp;
<a href="index.php?p=top&q=best">best movies</a>&nbsp;&nbsp;&nbsp;
<a href="index.php?p=top&q=worst">worst movies</a>&nbsp;&nbsp;&nbsp;
<!-- <a href="index.php?p=mostcomm">most commented</a>&nbsp;&nbsp;&nbsp; -->
<a href="index.php?p=http">movies on the web</a>&nbsp;&nbsp;&nbsp;
</b>

<hr size="1" width="">
<p>

<?php
include($page.".php");
?>

<p>
<hr size="1" width="">

<small>
<?php echo "Last modified: " . date ("F d Y H:i:s.", getlastmod()); ?> 
</small>

</body>
</html>

<?php
closeDB();
?>
