product fltk
    id "Fast Light Tool Kit, 1.0.1"

    image sw
        id "FLTK Execution Environment, 1.0.1"
        version 010001000

        subsys eoe default
            id "FLTK - Execution-Only Environment, 1.0.1"
            exp fltk.sw.eoe
        endsubsys

        subsys dev default
            id "FLTK - Development Environment, 1.0.1"
            exp fltk.sw.dev
            prereq
            (
            	fltk.sw.eoe 010001000 010001999
            )
        endsubsys
    endimage

    image man
        id "FLTK Documentation, 1.0.1"
        version 010001000

        subsys eoe default
            id "FLTK - Development Manuals, 1.0.1"
            exp fltk.man.dev
        endsubsys
    endimage
endproduct
