product fltk
    id "Fast Light Tool Kit, 1.0.4"

    image sw
        id "FLTK Execution Environment, 1.0.4"
        version 010004000

        subsys eoe default
            id "FLTK - Execution-Only Environment, 1.0.4"
            exp fltk.sw.eoe
        endsubsys

        subsys dev default
            id "FLTK - Development Environment, 1.0.4"
            exp fltk.sw.dev
            prereq
            (
            	fltk.sw.eoe 010004000 010004999
            )
        endsubsys
    endimage

    image man
        id "FLTK Documentation, 1.0.4"
        version 010004000

        subsys eoe default
            id "FLTK - Development Manuals, 1.0.4"
            exp fltk.man.dev
        endsubsys
    endimage
endproduct
