
Parsing the Buffer and Handling the Redirections 

Modified from the previous lab, we have a tokenize 
function that parses the buffer taking into account 
each token could be a word or a redirect symbol, in 
addition to the parse function that calls the 
tokenize helper; the tokens and argv array have the 
same number of tokens, but the argv contains strictly 
the file name instead of the file path. In order to make 
sure that our program reads in the input of the user 
correctly, we modified the tokenize function to be 
able to identify and normally parse redirects as well. 
This was done by adding variables like redirect_count_in 
and redirect_count_out to represent the number of “<” 
and “>” symbols there are, along with two more variables 
representing the index of where they appear in the buffer. 
This helped us identify any potential errors in the main(), 
where we make sure to throw an error if there is more than 
one of the same redirection in or out symbol in the same 
user input line. In addition to the tokens and argv 
array, we have the cleaned_tokens and cleaned_argv 
arrays, both the results of clean_buffer(), which removes 
the redirection symbols and the following token after 
an identified redirection symbol (idx + 1). 

Implementing the Built in Commands 

For all four of the commands, we had helper functions 
outside of the REPL. Each helper function was made to 
do error checks on any invalid inputs and then create 
the built-in functions using the system calls.

REPL, Forking, and Execv

The REPL in our main consists of a large while loop 
that runs indefinitely until a return statement is 
called. First, there is a check to see if the PROMPT 
macro was used. Then, we read the input and use a 
variable named “check” that helps the program represent 
the result of reading in the buffer; for example, if 
check returns a 0, this means that the entire buffer 
was successfully read in and the last token has been 
reached. In the normal case that the while loop is 
still reading the buffer, the parse function is 
called on this buffer. Then shortly after error checking 
for an invalid number of redirects or arguments, the 
cleaned_tokens and cleaned_argv arrays are saved in memory. 

Each in a conditional, the 4 built in system calls are 
identified here, and the respective helper functions are 
called on the cleaned_tokens. In the case that the user 
command is not one of any of the 4 built in commands, then 
we know that we must handle other function calls. Thus, in 
the else conditional, we forked the process so that the 
parent process waits for the child process to execute a 
specific command; this is also where the redirection is 
handled. In the case that a “<” is found, after error 
checking for any errors with the STDIN not closing 
properly, the correct file (found through index of 
redirection symbol index + 1) is opened with O_RDONLY 
and O_CREAT flags. On the other hand, when a “>” or “>>” 
is found, the same error checking is done but with the 
STDOUT, and the correct file is opened with O_WRONLY, O_CREAT, 
and O_TRUNC/O_APPEND flags. After redirections are handled, 
the execv is called and executes the command; the lines 
directly after this call to execv represent the error 
checking, as it will never reach those lines unless the execv fails.

Handeling Signals
To handle signals, we had to make sure that just the foregrorund 
process received user sent signals (not the actual
shell). To do this, we started off by ignoring SIGINT, SIGTSTP, 
and SIGTTOU. Then, we set each of these signals back to their default
within the child process. Also, to handle signals with multiple
processes, we give cotnrol to ur terminal.

Multiple Jobs
To account for multiple jobs appening within the shell, we 
use the job_list to keep track of all the bg jobs and suspended
fg jobs that are running. We update this list when a job changes
state. You can track the list by using the jobs command. Additionally,
under certain conditons, when a bg job chnages state, we somtimes
pritn messages to standard output.

Reaping
We handle all of our reaping within one function. This function
ensures that we handle jobs that have changes state sufficently.
Our reap function hendles when a process is stopped by signal,
terminated normally, terminated by signal, or continued. We
print the apporopiate messages and update the job lists accrodingly.

fg and bg
fg and bg are built-in commands that allow the user to bring
a background job to the foreground and  resume a background job
respectively. We use kill ot send signals to these proceses
and update our job list accordingly.

Work Distribution
We worked together throughout the entire process rather 
than dividing and conquering; we often used one person’s 
laptop to code together and frequently switched off. 
By writing a concrete plan and outline of how we want to 
complete the project, we were able to finish in an efficient
manner. After writing base code and getting a general 
program down, we worked on debugging specific issues together,
and if we were unable to work on debugging issues together 
due to time conflicts, we sent each other rundowns through 
thorough communication of how we debugged/what the issue was. 