node wajicup.js ../source/dos.c dos.o
node wajicup.js -stacksize 524288 -ld dos.o ../source/burn.c burn.js
node wajicup.js -stacksize 524288 -ld dos.o -args -embed edit.c ../source/edit.c ../source/edit.c edit.js
node wajicup.js -stacksize 524288 -ld dos.o ../source/julia.c julia.js
node wajicup.js -stacksize 524288 -ld dos.o ../source/mandelbrot.c mandelbrot.js
node wajicup.js -stacksize 524288 -ld dos.o ../source/plasma.c plasma.js
node wajicup.js -stacksize 524288 -ld dos.o -embed files/raycast/ ../files/raycast/ ../source/raycast.c raycast.js
node wajicup.js -stacksize 524288 -ld dos.o -embed files/rotozoom.gif ../files/rotozoom.gif ../source/rotozoom.c rotozoom.js
node wajicup.js -stacksize 524288 -ld dos.o -embed files/doom.mus ../files/doom.mus -embed files/simon.mid ../files/simon.mid -embed files/cfodder.mod ../files/cfodder.mod -embed files/soundcard.wav ../files/soundcard.wav -embed files/doom.op2 ../files/doom.op2 ../source/sound.c sound.js
node wajicup.js -stacksize 524288 -ld dos.o -embed files/stranded/ ../files/stranded/ ../source/stranded.c stranded.js
node wajicup.js -stacksize 524288 -ld dos.o ../source/tracker.c tracker.js
node wajicup.js -stacksize 524288 -ld dos.o -embed files/tunnel.gif ../files/tunnel.gif ../source/tunnel.c tunnel.js
node wajicup.js -stacksize 524288 -ld dos.o -embed files/C1W.gif ../files/C1W.gif -embed files/D1.gif ../files/D1.gif ../source/voxel.c voxel.js
