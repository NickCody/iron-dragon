
#include "users.h"
#include <string.h>

// =---------------------------------------------------------------------------
// Local Variable Declarations
// =---------------------------------------------------------------------------
char* Administrator_Usernames[] = 
{ 
   "NickAdmin",
   "MartinAdmin",
   "AlexAdmin",
   "GeorgeAdmin",
   "BernieAdmin",
    0
};

char* Beta_Tester_Usernames[] = 
{
   "Nick",
   "Myacks",
   "Martin",
   "szinger",
   "Rhelik",
   "ZombieLord",
   "Tamlyth",
   "AlStiles",
   "bannockburn",
   "BJoughin",
   "CabooseRider",
   "JoD",
   "Cthulhu",
   "Blackhawk",
   "Mantis",
   "dea3per",
   "vir4030",
   "Padre",
   "Kuragara",
   "Grond",
   "EvilBeard",
   "Ghostgames",
   "cking",
   "Ironwolf",
   "Jim Killjoy",
   "johnf",
   "Lampekapje",
   "Wedge",
   "PaulaMatuszek",
   "melkor",
   "mooncrow",
   "ojowyn",
   "o'ironside",
   "Buws",
   "slg",
   "tspencer",
   "kagato",
   "helix",
   "barticus",
   "ruato",
   "tbone",
   "nick1",
   "Ranger",
   "RMulford",
   "BPhillips",
   "NickAdmin",
   "NickTest33",
   "railman",
   "William",
   "tj14",
   "MartinAdmin",
   "Cyrano",
   "railtester",
   "irondragon",
   "jlake",
   "SThum",
   "rtownsend",
   "dchinnery",
   "Jonflagg",
    0
};


// =---------------------------------------------------------------------------
// U s e r _ I s _ A d m i n i s t r a t o r
//
// Returns: 1 on success (user is admin)
//          0 otherwise
//
// =---------------------------------------------------------------------------
int User_Is_Administrator ( const char* Username )
{
   int i = 0;
   while ( Administrator_Usernames[i] )
   {
      if ( strcmp ( Username, Administrator_Usernames[i] ) == 0 )
         return 1;
      i++;
   }

   return 0;
}

// =---------------------------------------------------------------------------
// U s e r _ I s _ B e t a _ T e s t e r
// =---------------------------------------------------------------------------
int User_Is_Beta_Tester ( const char* Username )
{
   int i = 0;
   while ( Beta_Tester_Usernames[i] )
   {
      if ( strcmp ( Username, Beta_Tester_Usernames[i] ) == 0 )
         return 1;
      i++;
   }

   return 0;
}

// =---------------------------------------------------------------------------
// U s e r _ I s _ B a c k _ E n d _ O p e r a t o r
// =---------------------------------------------------------------------------
int User_Is_Back_End_Operator ( const char* Username )
{
   return User_Is_Administrator ( Username ) ||
          User_Is_Beta_Tester ( Username );
}


