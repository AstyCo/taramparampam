#ifndef GL_CHECK_MACRO_H
#define GL_CHECK_MACRO_H

#ifdef _DEBUG
    #define GL_CHECK(stmt) do { \
            stmt; \
            GLenum err = glGetError();\
            if (err != GL_NO_ERROR)\
            {\
                QString errorName;\
                switch (error) {\
                case GL_NO_ERROR:\
                    errorName = "GL_NO_ERROR";\
                case GL_INVALID_VALUE:\
                    errorName = "GL_INVALID_VALUE";\
                case GL_INVALID_ENUM:\
                    errorName = "GL_INVALID_ENUM";\
                case GL_INVALID_OPERATION:\
                    errorName = "GL_INVALID_OPERATION";\
                case GL_STACK_OVERFLOW:\
                    errorName = "GL_STACK_OVERFLOW";\
                case GL_STACK_UNDERFLOW:\
                    errorName = "GL_STACK_UNDERFLOW";\
                case GL_OUT_OF_MEMORY:\
                    errorName = "GL_OUT_OF_MEMORY";\
                default:\
                    errorName = "UNDEFINED_GL_ERROR";\
                }\
                QString where = QString("at %1:%2 - for %3\n").arg(__FILE__).arg(__LINE__).arg(#stmt);\
                QByteArray whereArray = where.toLocal8Bit();\
                QString what = QString("OpenGL error %1").arg(errorName(err));\
                QByteArray whatArray = what.toLocal8Bit();\
                Q_ASSERT_X(false, whereArray.data(), whatArray.data());\
            }\
        } while (0)
#else
    #define GL_CHECK(stmt) stmt
#endif


#endif // GL_CHECK_MACRO_H
