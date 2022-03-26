#include<sys/socket.h>
#include<bits/stdc++.h>
#include<filesystem>
#include<dirent.h>
#include <arpa/inet.h>
#include <errno.h>
#include<sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#define CLIENT_MAX  10000
#define MAXBUF		1024
#define MAXIP		16
#define TRUE   1
#define FALSE  0
using namespace std;
string loopback;
char buffer1[MAXBUF];
char buffer2[MAXBUF];


class Client{
private:
    vector<string> downloads;
public:
    int client_id;
    int in_port;
    int unique_private_id;
    int im_neighbours;
    vector<pair<int,int>> neighbours;
    int no_files;
    vector<string> files;
    vector<string> myfiles;
   
};



/** Returns true on success, or false if there was an error */
bool SetSocketBlockingEnabled(int fd, bool blocking)
{
   if (fd < 0) return false;

#ifdef _WIN32
   unsigned long mode = blocking ? 0 : 1;
   return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
   int flags = fcntl(fd, F_GETFL, 0);
   if (flags == -1) return false;
   flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif
}




int main(int argc, char* argv[]){
// getting data   
    
    if(argc!=3){
        cout<<"Please provide 2 argumnets"<<endl;
        return 0;
    }
    ifstream indata;
    string config = argv[1]; 
    string dir_path = argv[2];

    Client C; // our client 

    // DATA From Config File
    

    // Directory contents
    

    // Open Config file
    indata.open(config);

    if(!indata) { // file couldn't be opened
        cerr << "Error: configfile could not be opened" << endl;
        exit(1);
    }
    vector<string> lines;
    int no_lines = 0;


    if (indata.is_open()){   //checking whether the file is open
        string tp;
        while(getline(indata, tp)){ //read data from file object and put it into string.
            lines.push_back(tp); // store line;
            no_lines++;
        }
        indata.close(); //close the file object.
    }

    istringstream iss0(lines[0]);
    iss0 >> C.client_id >> C.in_port >> C.unique_private_id;

    istringstream iss1(lines[1]);
    iss1 >> C.im_neighbours;

    istringstream iss2(lines[2]);
    for(int  i = 0; i< C.im_neighbours;i++){
        int a, b;
        iss2>>a>>b;
        C.neighbours.push_back(make_pair(a,b));
    }
    istringstream iss3(lines[3]);
    iss3>>C.no_files;
    for(int i = 0; i < C.no_files;i++){
        C.files.push_back(lines[4+i]);
    }

    // Getting my files

    struct dirent *d;
    DIR *dr;
    dr = opendir(argv[2]);
    if(dr!=NULL)
    {
        
        for(d=readdir(dr); d!=NULL; d=readdir(dr))
        {
            if(d->d_type == DT_REG ||d->d_type == DT_UNKNOWN ){
                cout<<d->d_name<<endl;
                C.myfiles.push_back(string(d->d_name));
            }
        }
        closedir(dr);
    }
    else
        cout<<"\nError Occurred!"<<endl;


     //Establish connection
     int opt = TRUE;
    int master_socket, addrlen, new_socket, 
    client_socket[C.im_neighbours],max_clients = C.im_neighbours, 
    activity,i ,valread, sd;
	struct sockaddr_in address;
    int max_sd;
    // set of socket discriptors
    fd_set readfds;
    fd_set writefds;
    // initialise client_socket to 0
    for(i = 0; i<max_clients; i++){
        client_socket[i]=0;
    }   
    
    //create master socket
    if((master_socket  = socket(AF_INET,SOCK_STREAM,0)) == 0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    else{
        printf("master socket doone");
    } 

    if(setsockopt(master_socket,SOL_SOCKET,SO_REUSEADDR,(char *)&opt, sizeof(opt))<0){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr =  INADDR_ANY;
    address.sin_port = htons(C.in_port);

    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", C.in_port);

    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while(TRUE){
        
        // clear the socket sets
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);

        FD_SET(master_socket,&readfds);
        FD_SET(master_socket,&writefds);   
        int on = 1;
        if (ioctl(master_socket, FIONBIO, &on) < 0) {
                perror("ioctl F_SETFL, FNDELAY");
                exit(1);
            }

        //add child sockets to set
        for(i = 0; i< max_clients; i++){
            sd = client_socket[i];
            int on =1;
            if (ioctl(sd, FIONBIO, &on) < 0) {
                perror("ioctl F_SETFL, FNDELAY");
                exit(1);
            }       
            // if valid socket descriptor then add to read list
            if(sd>0){
                FD_SET(sd, &readfds);
                FD_SET(sd, &writefds);
            }
            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
				max_sd = sd; 
        }
    

    }