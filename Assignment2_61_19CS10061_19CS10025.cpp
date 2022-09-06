#include <iostream>
#include <cstdlib>
#include <csignal>
#include <errno.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <map>
#include <dirent.h>
#include <unistd.h>             // syscalls
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_CHAR 100
using namespace std;

// sig_atomic_t signalled = 0;
vector<string>history;
char getch(void);
string filename("h.txt");
void run_shell();

void search_word(string word){
    vector <string> files;
    cout<<"\ngetting dir"<<endl;
    struct dirent *d;
    DIR *dr;
    dr = opendir("./");

    while ((d = readdir(dr)) != NULL) {
        std::string fname = d->d_name;
        files.push_back(fname);
    }
    int j=files.size();
    int flag=0;
    vector <string> substring;
    vector <string> r;
    istringstream iss(word);
    for (string s; iss >> s; )
        r.push_back(s);

    if (r.size() > 1){   
        string str = r.back();
        for(int i=0;i<j;i++){
            if(str==files[i]){
                flag=1;
                for(int i = 0; i < r.size()-1; i++)
                    cout << r[i]<<" ";
                cout<<files[i]<<endl;
            }
        }
        int m=0;
        if(flag==0){
            for(int i=0;i<j;i++){
                if(files[i].find(str)!=string::npos){
                    m++;
                    cout<<m<<" "<<files[i]<<endl;
                    substring.push_back(files[i]);

                }
            }
        }
    }

    else{
        for(int i=0;i<j;i++){
            if(word==files[i]){
                flag=1;
                cout<<files[i]<<" "<<endl;
                return;
            }

        }

        int m=0;
        if(flag==0){
            for(int i=0;i<j;i++){
                if(files[i].find(word)!=string::npos){
                    m++;
                    cout<<m<<" "<<files[i]<<endl;
                    substring.push_back(files[i]);

                }

            }
        }
    }

    if(substring.size() > 1){
        cout<<"Select a number:"<<endl;
        int number;
        cin>>number;
        for(int i = 0; i < r.size()-1; i++)
                cout << r[i]<<" ";
        cout<<substring[number-1]<<endl;
    }
      else {
        if(r.size()==1){
             cout << r[0]<<" ";
            cout<<substring[0]<<endl;
        }
        else if(r.size()>1)
        {
        for(int i = 0; i < r.size()-1; i++)
                cout << r[i]<<" ";
            cout<<substring[0]<<endl;
        }

        else 
            cout<<"No such file exists"<<endl;
    }
    return;
}

void search_history(){
    string word;
    cout<<"Enter a word to search:\n";
    getline(cin,word);
    ifstream  fp;
    fp.open(filename);
    int flag=0;
    string line;
    //getline(cin,word);
    unsigned int curLine = 0;
    while(getline(fp, line)) { 
            curLine++;
            if (line.find(word) != string::npos) {
                flag = 1;
                cout << "found: " << word  <<" line: " << curLine << endl;
            }
    }
    if(flag==0){
        cout<<"NOT FOUND!!!!"<<endl;
    }
}


void saveCommand(string sentence){
    string save(sentence);
    history.push_back(save);
}


void save_history( ){
    fstream fp;
    fp.open(filename,std::ios_base::app | std::ios_base::in);    
    if (history.empty()) {
        cout << "command history is empty" << endl;
    }     
    else {
        if(fp.is_open()){
            int i=history.size()-1;
            fp << history[i] << endl;
        }
    }
 }
   

void displayHistory(){
    ifstream fp;
    fp.open(filename);

    string line;
    //getline(cin,word);
    unsigned int curLine = 0;
    while(getline(fp, line)) { 
         curLine++;
         if(curLine>1000)
            break;

         cout<<curLine<<" "<<line<<endl;
    }
}



void signal_handler(int signal_num){
    signal(SIGTSTP, signal_handler);
    cout << "\nThe interrupt signal is (" << signal_num<< "). \n";
    pid_t pid = getpid();
    kill(pid, SIGCONT);
    run_shell();
}

void execute_command(string line, int in_fd, int out_fd) {
    istringstream iss(line);
    // cout<<line<<endl;
    
    vector<string> tokens;
    pid_t child_pid;
    int wait_flag = 1;
    int l =0, r=0, start=0,end=0;
    string read_file, write_file;
    
    copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(tokens));

    for(unsigned int i=0;i<tokens.size();i++){
        // cout<<tokens[i]<<" ";
    }
    for(unsigned int i=0;i<tokens.size();i++){
        if(tokens[i]=="<"){
            l = 1;
            read_file = tokens[i+1];
        }
        if(tokens[i]==">"){
            r=1;
            write_file = tokens[i+1];
        }
        if(tokens[i]=="&"){
            start=1;
        }
        if(tokens[i]=="history"){
            displayHistory();      
        }
        if(!l&&!r&&!start)
            end++;
    }
    const char **argv = new const char *[end+1];
    for (int i = 0; i < end; i++){
        argv[i] = tokens[i].c_str();
    }
    argv[end] = NULL;
    wait_flag = !start;
    // cout<<argv[0]<<" "<<end<<endl;
    if(strcmp(argv[0], "multiWatch") == 0){
        for(int i=1; i<end; i++){
            child_pid = fork();
            if (child_pid < 0) {
                perror("Fork failed!!");
                exit(1);
            }
            if ((child_pid == 0)){
                pid_t pid = getpid();
                string s = ".temp." + (pid) ;
                string t =  ".txt";
                s += t;
                // cout<<string(pid)<<endl;
                int fd = open(s.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
                cout<<"killing.."<<i<<endl;
                kill(pid, SIGTERM);
            }
            cout<<"done"<<endl;
        }
    }
    if(strcmp(argv[0], "cd") == 0){
        // cout<<"cd....."<<endl;
        if (argv[1] == NULL){
            fprintf(stderr, "shell: expected argument to \"cd\" command!");
        } 
        else{
            if (chdir(argv[1]) != 0){
                perror("shell");
            }
        }
    }
    else{
        child_pid = fork();
        if (child_pid < 0) {
            perror("Fork failed!!");
            exit(1);
        }
        if ((child_pid == 0)){
            signal(SIGINT, SIG_DFL);
            if(r)
                out_fd = open(write_file.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
            
            if(out_fd!=1) {
                dup2(out_fd, 1);
            }
            if(l) 
                in_fd = open(read_file.c_str(), O_RDONLY);
            
            if(in_fd!=0) {
                dup2(in_fd, 0);
            }
            
            execvp(tokens[0].c_str(), (char **)argv);
            exit(0);
        }
        
        else{
            if(wait_flag){
                wait(NULL);
                if(in_fd!=0)
                    close(in_fd);
                if(out_fd!=1)
                    close(out_fd);
            }
            else{
                cout<< "[Process...] Background\n";
            }
        }
    }
    return;
}

string get_command(){
    string sentence;
    while(true){
    char c = getch();
    if(c == '\t') 
        search_word(sentence); 
    else if(c == 18) { 
        search_history();
        sentence.clear();
        break;
    }
    else if(c == 127) { // Backspace
        if(sentence.length()>0){
            sentence.pop_back();
            fputs("\b \b",stdout);
        }
    }      
    else if (c == '\n') { 
        putchar(c);
        break;
    }
    else if (c >= 32 && c <= 127) { // Printables
        putchar(c);
        sentence.push_back(c);
    }
    }
    return sentence;
}


void run_shell(){
    string line;
    
    while (true){
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, signal_handler);
        
        cout << "Assgn2@19CS10061_19CS10025$ ";
        // getline(cin, line);
        line = get_command();             // Reading the input line 
        // cout<<"Evaluating '"<< line<<"'"<<endl;
        if (line == "") {      /* Handle empty commands */
            continue;
        }

        if (line == "exit" || line == "quit"){
            cout<<"Exitting Shell..."<<endl;
            pid_t pid = getpid();
            kill(pid, SIGTERM);
            break;
        }
        
        vector<string> commands;
        int start_cmd = 0, end_cmd = 0;
        for(int i=0;line[i]!='\0';i++){
            if(line[i]=='|') {
                commands.push_back(line.substr(start_cmd, end_cmd-start_cmd));
                start_cmd = end_cmd = i+1;
            }
            else
                end_cmd++;
        }
        commands.push_back(line.substr(start_cmd, end_cmd-start_cmd));

        int no_pipes = commands.size()-1;
        int **pipes = new int *[no_pipes];
        for(int j=0;j<no_pipes;j++){
                pipes[j]=new int[2];
                pipe(pipes[j]);
        }

        for(unsigned int i=0;i<commands.size();i++) {
            int in_fd = 0, out_fd = 1;
            int pre_out = -1, next_in = -1;
            if(i>0) {
                in_fd = pipes[i-1][0];
                pre_out = pipes[i-1][1];
            }
            if(i<commands.size()-1) {
                out_fd = pipes[i][1];
                next_in = pipes[i][0];
            }
            saveCommand(commands[i]);
            save_history();
            execute_command(commands[i], in_fd, out_fd);
        }
       fflush(stdin);
       fflush(stdout);
    }
    return;
}


int main(){
    cout << "----------  Group 61 Shell ----------\n\n";
    run_shell();
    return 0;
}

char getch(void){
    int ch;
    struct termios oldt;
    struct termios newt;
    tcgetattr(STDIN_FILENO, &oldt); /*store old settings */
    newt = oldt; /* copy old settings to new settings */
    newt.c_lflag &= ~(ICANON | ECHO); /* make one change to old settings in new settings */
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); /*apply the new settings immediatly */
    ch = getchar(); /* standard getchar call */
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); /*reapply the old settings */
    return ch; /*return received char */
}