
#ifndef __USERS_H__
#define __USERS_H__

#ifdef __cplusplus
extern "C"
{
#endif

int User_Is_Administrator         ( const char* Username );
int User_Is_Beta_Tester           ( const char* Username );
int User_Is_Back_End_Operator     ( const char* Username );

#ifdef __cplusplus
}
#endif

#endif // USERS_H__
