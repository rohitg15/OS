#include<stdio.h>
#include<stdlib.h>
#include<cstring>
#include<algorithm>
#include<map>
#include<string>
#include<vector>
#include<iostream>


using namespace std;


#define FOR(i,x,n) for(i=0;i<x;i++)

map<string,int> hash_words;
vector<string> words;


bool custom_sort(string s1,string s2)
{
  if (hash_words[s1] < hash_words[s2])
    {
      return true;
    }
  else if (hash_words[s1] > hash_words[s2])
    {
      return false;
    }
  else
    {
      int i=0,j=0;
      while(i<s1.size() && j < s2.size())
	{
	  if (s1[i] < s2[j])
	    {
	      return true;
	    }
	  else if(s1[i] > s2[j])
	    {
	      return false;
	    }
	  i++,j++;
	}
      if (i >= j)
	{
	  return false;
	}
      return true;
    }
  
}

void parse_words(char *buf,int size)
{
  if (!buf || size <= 0)
    {
      return;
    }

  //protect the initial value of buf
  
  int i=0;
  buf[size] = 0x0;
  //remove leading spaces
  while(i<size && (*buf == ' '))
    {
      buf++;
      i++;
    }

  if (i >= size)
    return;

  //  char dest[size];
  
  while(*buf)
    {
      int pos = 0;
      // find the first occurence of a space
      char *pos_ptr = strchr(buf,' ');
      if (!pos_ptr)
	{
	  // no space character was found
	  pos = size;
	}
      else
	{
	  pos = pos_ptr - buf;
	}

      
	  buf[pos] = 0x0;      
    
      string str(buf);
      if (hash_words.find(str)  != hash_words.end())
	{
	  hash_words[str]++;
	}
      else
	{
	  hash_words[str] = 1;
	  words.push_back(str);
	}


      //reset buffer positions
      buf += pos + 1;
      size = size - pos - 1;
      i = 0;
      
        //remove leading spaces
      while(i<size && (*buf == ' '))
	{
	  buf++;
	  i++;
	}
      //adjust new size of the string. we have skipped i positions in the string now
      
      size -= i;
      i = 0;
      
    }
 }

int main(int argc,char *argv[])
{

  if (argc != 2)
    {
      fprintf(stderr,"Error: provide a file name\n");
      exit(0);
    }

  FILE *fp = NULL;
  fp = fopen(argv[1],"r");
  if (!fp)
    {
      fprintf(stderr,"could not open file descriptor to %s",argv[1]);
      exit(0);
    }

  char buf[64];
  memset(buf,0,sizeof(char)*64);
  

  while(!feof(fp))
    {
      //read contents till end-of-file is reached
     
      char *res = fgets(buf,sizeof(buf),fp);
      if (res)
	{
	  //here the input buffer also has the '\n' in addition to the NULL character. So we do not pass strlen(buf) + 1
	  parse_words(buf,strlen(buf) - 1);
	}
    }
  
  
  //sort the words by count. if count matches then sort by alphabetical order
    sort(words.begin(),words.end(),custom_sort);
  
    //print vector
  for(vector<string>::iterator it = words.begin();it != words.end(); it++)
    {
      cout<<*it<< " :  "<<hash_words[*it]<<endl;
    }

  fclose(fp);

  return 0;
}