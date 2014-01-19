ScummDS
=======
ScummDS is an attempt to get (especially Humongous Entertainment) scumm games to run on the DS. I know there is a DS port of ScummVM, but it can't run 640x480 games, because it's not optimized for the DS.

The interpreter part is heavily based on ScummVM, but optimized for the DS. <b>This project has no relation to ScummVM at all!</b>

Because I used a lot of their code, <b>I want to credit all people that worked on ScummVM</b> (mainly the scumm engine) for <b>the great work they did on reverse engineering</b> all of this difficult stuff!

<img src="http://florian.nouwt.com/forum/get.php?id=30" align="right"/>
<h3>Current State</h3>
Currently I got some games to run. Actors work, but not very good and it is all a little bit slow. Also, there are still opcodes that are not implemented. Object stuff is implemented, but not drawn.

<h3>Supported Games</h3>
The games listed here do NOT run fully correctly (since the interpreter is just not finished), but atleast they boot past the HE logo and show their first location you can actually move your mouse. I am not sure if language matters, but it is possible that the HE version differs between languages.<br>
* <b>Freddi Fish and Luther's Maze Madness</b> (Dutch, Freddi Fish en Loebas' Dolle Doolhof)
* <b>Pajama Sam</b> (Dutch)
* <b>Pajama Sam 2</b> (Dutch) - Skip the intro cut-scene for better results
* <b>Pajama Sam 2 Demo</b> (Dutch)
* <b>Pajama Sam's Sock Works</b> (Dutch, Pajama Sam Sokkensoep)

<h3>Usage</h3>
Use <a href="https://github.com/Gericom/HEPacker">HEPacker</a> to pack your game and change the path in arm9/source/main.cpp. After that, just compile and run on your ds.
