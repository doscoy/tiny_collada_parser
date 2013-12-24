
#ifndef TC_SAMPLE_MULTIPLATFORM_HPP_INCLUDED
#define TC_SAMPLE_MULTIPLATFORM_HPP_INCLUDED



#ifdef _MSC_VER // msvc
    #include <gl/glut.h>
#elif   //mac
    //  mac用
    #include <GLUT/GLUT.h>
#endif

#endif // TC_SAMPLE_MULTIPLATFORM_HPP_INCLUDED

