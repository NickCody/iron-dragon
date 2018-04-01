// =---------------------------------------------------------------------------
// c o m m a n d _ l i n e . h
// 
//   (C) 1999-2000 - Martin R. Szinger, Nicholas Codignotto.
//
//
//   DESCRIPTION
//   -----------
//
//   WIN32 IMPLEMENTATION NOTES
//   --------------------------
//
//   UNIX IMPLEMENTATION NOTES
//   --------------------------
//
//   DATE        WHO   REVISION
//   ---------   ---   --------------------------------------------------------
//   99 Aug 28   nic   Created.
//

extern char                   Version[];
extern char                   Data_Directory[MAX_PATH];
extern char                   Log_Directory[MAX_PATH];
extern char                   Log_Filename[MAX_PATH];
extern char                   Cmd_String[];
extern uint_16                TCP_Port_IDPCD;
extern uint_16                Max_Clients;

#ifdef UNIX

extern bool                   Run_As_Daemon;        
extern char                   Config_Filename[];

#endif

extern bool                   Create_Databases;

bool Parse_Command_Line ( int argc, char** argv );
bool Parse_Configuration ( const char* file );
void Display_Usage( void );
