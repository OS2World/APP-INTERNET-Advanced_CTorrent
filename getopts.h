#ifndef GETOPTS_H
#define GETOPTS_H

#ifdef __cplusplus
extern "C" {
#endif

extern int   OptNum;
extern int   ParamNum;
extern int  ArgIdx;

int getopts(int argc, char **argv, char **OptArr);

#ifdef __cplusplus
}
#endif
#endif
