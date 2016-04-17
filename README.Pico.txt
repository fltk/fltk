
This documentation will explain how to quickly port FLTK to a new
platform using the Pico driver system. For now, only the sample 
SDL Pico driver on OS X compiles and barely runs.

> mkdir build
> mkdir XcodeSDL
> cmake -G Xcode -D OPTION_APPLE_SDL=ON ../..

tl;dr - the recent commit should be transparent to all developers 
on other platforms. On OS X, the CMake setup add the option OPTION_APPLE_SDL=ON 
that will run FLTK on top of SDL, which in turn runs on top of the 
new "Pico" driver set.

The greatest help I can get form the core developers is to continue 
to refactor the platform specific functionalities into the drivers.

---

OK, long version. I know that I am repeating myself and I don't expect 
those of you who "got it" weeks ago to read this again. Writing this 
down is also for me to avoid losing track ;-)

Goal 1: find all the stuff that must still go into drivers
Goal 2: have a base driver for all future porting efforts
Goal 3: make porting fun with early gratification to the coder
Goal 4: extensively document a sample port (SDL), and how to improve it
Goal 5: use SDL as a base library, thereby porting FLTK to iOS and Android


This is the start of a new driver, named "Pico". "Pico" is here to 
implement what I called the minimal driver.

"Pico" implements (eventually) a complete set of drivers. The drivers 
will be limited in functionality, but they will be good enough to 
allow basic UI's with most FLTK widget types. 

If "Pico" compiles without any "USE_PICO" or similar defines, we 
have reached goal 1.

"Pico" will implement all driver functionalities "in good faith", 
meaning, in a way that FLTK runs without crashing. Only very very 
basic functions are not implemented. A driver that derives form "Pico" 
needs only to implement those missing functions, thereby reaching goals 
2 and 3. As far as I can tell, those will be:

- open a single fixed size window
- setting a pixel in a color inside that window
- polling and waiting for PUSH and RELEASE events an their position

By implementing these three simple functions in the SDL driver, 
"test/hello" and quite a bunch of other tests will run (yes, they 
will be slow, but the will work).

This will give the person who is porting FLTK to their platform a 
very early confirmation that their build setup is working and a 
very early gratification, reaching goal 3.

Obviously, SDL is much more powerful, and by the nature of the 
virtual functions in the driver system, we can incrementally add 
functionality to the SDL driver, and document this incremental 
nature, reaching goal 4.

If we do all this right, we have goal 5.

If SDL is too big or complex for, say, Android, we can simply start 
a new native Android driver set by implementing the three functions 
mentioned above, and then go from there.


- Matthias



