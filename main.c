#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <ctype.h>

#include <locale.h>

int isAllDigit(char * word) { //Checks if the word contains only numbers.
  for (int i = 0; i<strlen(word); i++) {
    if (isdigit(word[i]) == 0) {
      return 0;
    }
  }
  return 1;
}

void intChecker(char * word, int isNegative, FILE * f, FILE * w) { //Checks if the word is an integer.

  {
    if (isAllDigit(word)) {
      if (isNegative) {
        fprintf(w, "IntConstant %s\n", word - 1); /*For negative numbers We give the number as a parameter but not '-'. 
                                                  "word - 1" makes it negative.
                                                  For example if the word is -8 we give 8 as a parameter to this function and now 
                                                   in this function "word - 1" and that makes it -8.*/
        
      } else {
        fprintf(w, "IntConstant %s\n", word);
       
      }
    } else { //If the word starts with '-' or a digit but doesn't continue with digit that gives an error.
      if (isNegative)
        fprintf(w, "Invalid word %s\n", word - 1);
      else
        fprintf(w, "Invalid word %s\n", word);
      
    }
  }
}

void readString(FILE * f, FILE * w, char ch) { //Reads strings
  int i = 0;
  ch = fgetc(f);
  char * string = malloc(100);

  while (ch != '"' && ch != EOF) { //Adds all characters to variable string until quotation marks

    *(string + i) = ch;
    i++;
    ch = fgetc(f);

  }
  *(string + i) = '\0'; //Then close the string
  
    if (ch != EOF) {
      fprintf(w, "StringConstant %s\n", string);
      
    } else {
      
      fprintf(w, "String started but didn't terminate!\n");
    }

  

  free(string);
}
int isOpened = 0; //If loop is started this will be 1
int isClosed = 0; //If loop is closed this will be 1
int isCommentClosed = 0;
void detector(char ch, FILE * f, FILE * w) { //Takes a character, checks it and gives the appropriate output.
  
  if (!(isalnum(ch)) && ch != '-') //This function is not for alphanumeric characters or '-'.
    switch (ch) {

      case '"': //If a string is started, the string reading function is called.
      readString(f, w, ch);
      break;
      case ',':
        fprintf(w, "Seperator\n");
        break;
      case '{':
        
        while ((ch = fgetc(f)) != '}') {  //Goes to the end of the comment.
            if(ch == EOF){ //If comment is not closed until the end of file, gives an error.
                fprintf(w, "File ended before the comment is completed!");
                break;
            }
        }
        if(ch == '}') isCommentClosed = 1;
        break;
      case '}': //If comment bracket closes without opening, gives an error.
        fprintf(w, "Comment closed with no start!\n");
        break;
      case ' ':
      case '-':
      case '\t':
      case '\n':
        break; //Skips the 4 characters above.
      case '[':
        fprintf(w, "OpenBlock\n");
        isOpened = 1;
        break;
      case ']':
        if(!isOpened){ //If loop block hasn't been opened before, gives an error.
          fprintf(w, "Error: Block hasn't been opened before!\n");
        } else{
          fprintf(w, "CloseBlock\n");
          isClosed = 1;
        }
        break;
      case '.':
        fprintf(w, "EndOfLine\n");
        break;
      default:
        if (ch != '_') 
          fprintf(w, "Unrecognized Character (%c)\n", ch);
        ch = fgetc(f);
        if (isalnum(ch)) {          
          fprintf(w, "Invalid identifier word\n");
          while (isalnum(ch) || ch == '_')
            ch = fgetc(f);
        }
        if (ch != EOF)
          fseek(f, -1, SEEK_CUR);
      }
}



int wordChecker(FILE * f, char * word, char ** keywords, FILE * w) //Finds mistakes in words.
{

  if (isdigit( * word) || * word == '-') {
    if ( * word == '-') //If first character is '-'
      if (isdigit( * (word + 1))) // Checks if a number comes after '-'
        intChecker(word + 1, 1, f, w); //Writes intconstant to file with intChecker function. Second parameter is 1 because number is negative
      else{ // If a non-digit character comes after '-' gives an error.
        fprintf(w, "Invalid word %s\n", word);
      }

    else //If the word is not negative
      intChecker(word, 0, f, w);
  } else { //If the word is not a number
      int iskeyword = 0;
      for (int i = 0; i < 10; i++) // Keyword control
      {
        char * key = keywords[i];;

        if (strcmp(key, word) == 0) {
          fprintf(w, "Keyword '%s'\n", keywords[i]);
          
          iskeyword = 1;
          break;
        }
      }
      if (!iskeyword) { //If the word is neither a number nor keyword
        if (isdigit(word[0]) || word[0] == '_') { //If the word starts with a digit or underscore
          fprintf(w, "Invalid identifier token (%s)\n", word);
          return 0;
        } else {
          int error = 0;
          for (int i = 0; i < strlen(word); i++) {
            if ((!isalnum((word[i])) && word[i] != '_')) {
              error = 1;
            }
          }
          if (!error) {
            fprintf(w, "Identifier '%s'\n", word);
            
          } else {
            fprintf(w, "Invalid identifier Token '%s'\n", word);
          
          }
        }
      }

    }

}

int main() {

  setlocale(LC_ALL, "Turkish");

  FILE * file;
  FILE * filetoWrite;

  char * keywords[] = {
      "add",
      "sub",
      "move",
      "to",
      "loop",
      "out",
      "int",
      "from",
      "times",
      "newline"
  };
    

  char * filename = malloc(100);
  fflush(stdin);
  printf("Enter source file name with its extension (for example, myscript.ba) : ");
  gets(filename);

  char ch;

  file = fopen(filename, "r");
  filetoWrite = fopen("myscript.lx", "w");

  if (file == NULL) {
      fprintf(filetoWrite, "\nFile couldn't open\n\n");
      return 0;
  } else {
      printf("\nFile Opened.\n\n");
  }

  while ((ch = fgetc(file)) != EOF) {
      detector(ch, file, filetoWrite);

      if (isalnum(ch) || ch == '-') {
        int chr = 0;
        char * word = malloc(100);
        while (ch != ' ' && ch != '\n' && ch != '\t' && ch != ',' && ch != '.') { //Each word is being created in this loop.
            *(word + chr) = ch;
            chr++;
            ch = fgetc(file);
            if (ch == EOF) {

            break;
            }
        }
        fseek(file, -1, SEEK_CUR); //Brings us to the index of the last character of the word and then will continue from this index.
        *(word + chr) = '\0'; //End of a word

        wordChecker(file, word, keywords, filetoWrite); //Reads and checks word.
        free(word);
      }

  }
  if(!isClosed && isCommentClosed){ //If loop block is not closed and comment block is closed, gives an error.
      fprintf(filetoWrite, "Error: Block not closed!");
  }

  fclose(file);
  fclose(filetoWrite);
    

    return 0;

}