Notes on adding new options to Flameshot
========================================

Let's add extPgmTerminal as a boolena:

1. Create the getter/setter in configHandler:

   src/utils/configHandler.h - `CONFIG_GETTER_SETTER(extPgmTerminal,setExtPgmTerminal,bool)`

2. Add the option to the file parser:

    src/utils/confighandler.cpp - `OPTION("extPgmTerminal"             ,Bool               ( false              ))`

3. Add widgets to src/config/*.cpp/h
   1. add member variable
   2. add init function
   3. connect signals to created function or lambda
   
   

