// The MIT License (MIT)
//
// Copyright (c) 2016, 2017, 2021 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// 7f704d5f-9811-4b91-a918-57c1bb646b70
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

//includes
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>


// creating defines
#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10    // Requirement 7: Modified max num arguments to
                                // 10 instead of 5 so our shell can support up to 
                                // 10 command line parameters

#define MAX_NUM_PIDS 15         // Requirement 11: created define MAX_NUM_PIDS
                                // as the we will only support the last 15 
                                // processes spawned by the shell

#define MAX_NUM_HISTORY 15      // Requirement 12: created as the history command 
                                // will only store up to the last 15 commands that 
                                // were entered by the user

int main()
{
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  // initializing pid index variable to take count of how many commands we have done
  // this will be useful for satisfying Requirement 11 as we will implement this count
  // variable to our history
  int pid_index = 0;

  // initializing pid counter variable that we will utilize for the for loop that will print out
  // the number of pids there are.. not print out up to 15 as some could be blank. 
  int pid_counter = 0;

  // Initializing pid array that will have an array filled with integers.
  pid_t pid_list[MAX_NUM_PIDS];

  // initializing int history index variable to take count of how many commands we have done
  // this will be useful for satisfying Requirement 12 as we will implement this count
  // variable to our history
  int history_index = 0;

  // initializing pid counter variable that we will utilize for the for loop that will print out
  // the number of history there are.. not print out up to 15 as some could be blank.
  int history_counter = 0; 

  // initializing thie history array that will be filled with the previous commands
  char *history_list[MAX_NUM_HISTORY];

  // allocating memory for the indexes of history_list array
  for(int i = 0; i < MAX_NUM_HISTORY; i++)
  {
    history_list[i] = ( char * ) malloc(MAX_COMMAND_SIZE); 
  }

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;                                
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;                                         

    char *working_str  = strdup( cmd_str );

    // essential history code
    // storing the recent command string into another variable called 
    // recent command and then assigning that command into the 
    // history index at a specific location
    char *recent_command = strdup(cmd_str);
    history_list[history_index] = recent_command;

    // incrememnting the two different counter and index variables
    history_counter++;
    history_index++;

    // initializing int variable that will hold the location of the history
    int historyLocation; 

    // if we surpass the max number of history commands we will reset
    // the history_index variable
    if (history_index >= MAX_NUM_HISTORY)
    {
      history_index = 0;
    }

    // Maxing out the history_counter if it reaches the max number
    // of history elements as we cannot display more than this amount.
    if (history_counter > MAX_NUM_HISTORY)
    {
      history_counter = MAX_NUM_HISTORY;
    }

    // checking if the first inputted character is an ! for the !n command
    if(working_str[0] == '!')
    {
      // creating an int variable that will hold the history number we want to go to
      // we subtract 1 from the atoi value as the code will print beginning from 1. 
      historyLocation = (atoi(&working_str[1]) - 1); 

      // this if statement will check if the history location will fall within the
      // boundary for the max and min. must have a history location to go to
      if (0 <= historyLocation && historyLocation <= history_counter)
      {
        // copying that string from the array into the working string
        // so it acts like were typing it in again
        strcpy(working_str, history_list[historyLocation]); 
      }
    }

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    // if we surpass the max number of pids we will reset the index variable
    if (pid_index >= MAX_NUM_PIDS)
    {
      pid_index = 0;
    }

    // Maxing out the pid counter variable to the MAX_NUM_PIDS so it will not
    // print more than the MAX_NUM_PIDS when calling that command
    if (pid_counter > MAX_NUM_PIDS)
    {
      pid_counter = MAX_NUM_PIDS;
    }

    // putting everything in an if statement so we can resolve the seg fault issue
    // as we would get issues if we would press enter with token[0] being null
    // having this if statement will allow us to run the code if the token[0] 
    // is not null, but i it is, we will simply do nothing. 
    if(token[0] != NULL)
    {
      // if statement that will check if the user wants to complete the cd command
      if (strcmp(token[0], "cd") == 0)
      {
        chdir(token[1]);
      }
      // if statment that will check if the user enters the exit or quit command
      // this will call the exit with the 0 code and end the program
      else if ((strcmp(token[0], "exit") == 0) || (strcmp(token[0], "quit") == 0))
      {
        exit(0);
      }
      // this command will check if the user types in listpids, and will proceed to 
      // list up to the last MAX_NUM_PIDS 
      else if ((strcmp(token[0], "listpids")) == 0)
      {
        // for loop that is utilized to go through the pid array and print out the
        // the MAX_NUM_PIDS most recent pids, this is where we use that pid_counter
        for(int i = 0; i < pid_counter; i++)
        {
          printf("\n%d:  %d", (i+1), pid_list[i]); 
        } 
        printf("\n"); 
      }
      else if ((strcmp(token[0], "history")) == 0)
      {
        // for loop that is utilized to go through the history array and print out the
        // the MAX_HISTORY_NUM of the most recent commands, this is where we use that counter
        for (int i = 0; i < history_counter; i++)
        {
          printf("%d:  %s", (i + 1), history_list[i]);
        }
      }
      else
      {
        // forking
        pid_t pid = fork();

        // essential pid code  
        // storing pids into array
        pid_list[pid_index] = pid;

        // increment the two different pid counters and indexes
        pid_counter++; 
        pid_index++; 

        if (pid == 0)
        {
          // Notice you can add as many NULLs on the end as you want
          int ret = execvp(token[0], &token[0]);
          if (ret == -1)
          {
            // checking if the value falls along the bounds of the history when using the 
            // ! command. if it is not a ! command it will simply go to the 
            // command not found error
            if (!(0 <= historyLocation && historyLocation < (history_counter-1)))
            {
              printf("Command not found in history.\n");
            }
            else
            {
              printf("%s: Command not found.\n\n", token[0]);
            }
            break; // utilizing break to ensure that we can break out of the loop
          }
        }
        else
        {
          int status;
          wait(&status);
        }
      }
      free( working_root );
    }
  }

  // freeing memory that we allocated for the history array
  for (int i = 0; i < MAX_NUM_HISTORY; i++)
  {
    free(history_list[i]);
  }

  return 0;
}
