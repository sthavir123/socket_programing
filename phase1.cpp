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
#include<thread>
#define CLIENT_MAX  10000
#define MAXBUF		1024
#define MAXIP		16
#define TRUE   1
#define FALSE  0
using namespace std;
string loopback;
char buffer1[MAXBUF];
char buffer2[MAXBUF];

int sendcount1;
int recvcount1;


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
    map<int,bool> connected;
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


// int SetAddress(string comp, struct sockaddr_in *Addr)
// {	
//     string s = "127.0.0.1:"+comp;





void cus_send(int new_socket,fd_set readfds,string mess){
    int on = 0;
    if (ioctl(new_socket, FIONBIO, &on) < 0) {
               perror("ioctl F_SETFL, FNDELAY");
               exit(1);
          }

    if( send(new_socket, mess.c_str(),mess.length(), 0) != mess.length() ) 
            {
                perror("send");
            }
    else{
        sendcount1++;
    }        
    
}

void cust_recv(int sd, fd_set readfds){
    int on = 0;
    if (ioctl(sd, FIONBIO, &on) < 0) {
               perror("ioctl F_SETFL, FNDELAY");
               exit(1);
          }
    if (FD_ISSET( sd , &readfds)) 
            {
                int valread,addrlen;
                struct sockaddr_in address;
            
                valread = read( sd , buffer1, 1024);
                
                if (valread == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    close( sd );
                }
                
                //Echo back the message that came in
                else
                {
                 //set the string terminating NULL byte on the end of the data read
                    if(buffer1[0]!='\0'){

                    char *token = strtok(buffer1, ":");
                    printf("connected to %s",token);
                    token = strtok(NULL, ":");
                    printf(" with unique id %s",token);
                    token = strtok(NULL, ":");
                    printf(" on port %s\n",token);       
                    
                    
                    buffer1[0] = '\0';
                    //cout<<buffer1<<endl;
                    recvcount1++;
                    }
                    
                }
            }
}


int main(int argc, char* argv[]){

sendcount1=0;
recvcount1=0;

// getting data   
    
    if(argc!=3){
        cout<<"Please provide 2 argumnets"<<endl;
        return 0;
    }
    ifstream indata;
    string config = argv[1]; 
    string dir_path = argv[2];

    Client C; // our client 
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
                //cout<<d->d_name<<endl;
                C.myfiles.push_back(string(d->d_name));
            }
        }
        closedir(dr);
    }
    else
        cout<<"\nError Occurred!"<<endl;
    
    sort(C.myfiles.begin(),C.myfiles.end());
    for(auto item: C.myfiles){
        cout<<item<<endl;
    }

    // initialising all connected to false
    for(auto item:C.neighbours){
        C.connected[item.first] = false;
    }

    // Establishing Connections
    int opt = TRUE;
    int master_socket, addrlen, new_socket, 
    client_socket[C.im_neighbours],client_in[C.im_neighbours],max_clients = C.im_neighbours, 
    activity,i ,valread, sd;
	struct sockaddr_in address;
    int max_sd=0;
    // set of socket discriptors
    fd_set readfds;
    fd_set writefds;
    
    // timeout 
    struct timeval timeout;      
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;   
  
    //info string
    string mess = to_string(C.client_id) +":" +to_string(C.unique_private_id)+ ":" + to_string(C.in_port);    

    // initialise client_socket to 0
    for(i = 0; i<max_clients; i++){
        client_socket[i]=0;
    }
    for(int i = 0; i < C.im_neighbours;i++){
        client_in[i]=0;
    }
    //create master socket
    if((master_socket  = socket(AF_INET,SOCK_STREAM,0)) == 0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
  
    //set master socket to allow multiple connections
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
    int on = 1;
    

    
    


    //printf("Listener on port %d \n", C.in_port);

    if (listen(master_socket, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    //puts("Waiting for connections ...");

    if (ioctl(master_socket, FIONBIO, &on) < 0) {
               perror("ioctl F_SETFL, FNDELAY");
               exit(1);
          }

    
   
    
    //int MAXTRY = 10;
    vector<thread> process1;
    vector<thread> process2;
    while(true){
        //MAXTRY--;
        process1.clear();

        //clear the socket set
        FD_ZERO(&readfds);

        int tpi=0;
        for(auto item:C.neighbours){
            if(C.connected[item.first]) {tpi++;continue;}
            struct sockaddr_in address1;
            address1.sin_family = AF_INET;
            address1.sin_addr.s_addr =  INADDR_ANY;
            address1.sin_port = htons(item.second);
            int sd =socket(AF_INET,SOCK_STREAM,0);
            if(sd>0){
                client_in[tpi] = sd;
                FD_SET(sd, &readfds);
                if(sd > max_sd){
                    max_sd  = sd;
                }
            }
            else{tpi++;continue;}
            
            if ( connect(client_in[tpi], (struct sockaddr *)&address1, sizeof(address1)) != 0 ){
             //do something   
            }
            else{
                C.connected[item.first] = true;
            }
            tpi++;

        }

        for(int i = 0; i < C.im_neighbours; i++){
            int sd = client_in[i];
            if(sd>0){
                FD_SET(sd, &readfds);
                if(sd > max_sd){
                    max_sd  = sd;
                }
            }
        }

        //add master socket to set
        FD_SET(master_socket, &readfds);
        if(master_socket > max_sd){
            max_sd  = master_socket;
        }
        
        
        for(i = 0; i< C.im_neighbours; i++){
            sd = client_in[i];
            int on =1;
            if (ioctl(sd, FIONBIO, &on) < 0) {
                perror("ioctl F_SETFL, FNDELAY");
                exit(1);
            }       
            // if valid socket descriptor then add to read list
            if(sd>0){
                FD_SET(sd, &readfds);
                if(sd > max_sd)
				{max_sd = sd;}
            }
            //highest file descriptor number, need it for the select function
             
        }


        //add child sockets to set
        for(i = 0; i< max_clients; i++){
            sd = client_socket[i];
                   
            // if valid socket descriptor then add to read list
            if(sd>0){
                FD_SET(sd, &readfds);
                if(sd > max_sd){
				    max_sd = sd;
                }
            }

            int on =1;
            if (ioctl(sd, FIONBIO, &on) < 0) {
                perror("ioctl F_SETFL, FNDELAY");
                exit(1);
            }
            //highest file descriptor number, need it for the select function
             
        }

        // wait for an activity with time out
        activity = select(max_sd+1,&readfds,NULL,NULL,NULL);
        //cout<<activity;
        if ((activity < 0) && (errno!=EINTR)) 
        {
            printf("select error");
            break;
        }
        //cout<<"here2"<<endl;


        if (FD_ISSET(master_socket, &readfds)) 
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                //exit(EXIT_FAILURE);
            }
            if (ioctl(new_socket, FIONBIO, &on) < 0) {
                perror("ioctl F_SETFL, FNDELAY");
                exit(1);
            }
            //inform user of socket number - used in send and receive commands
            //printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
           
            //send new connection greeting message
            
            process1.push_back(thread(cus_send,new_socket,readfds,mess));
            
             
            
             
            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++) 
            {
                //if position is empty
				if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    
                    break;
                }
            }

        }
        //else its some IO operation on some other socket :)
        for (i = 0; i < C.im_neighbours; i++) 
        {
            
            sd = client_in[i];
            
            if(!(sd>0)) continue;
            if (ioctl(sd, FIONBIO, &on) < 0) {
                perror("ioctl F_SETFL, FNDELAY");
                exit(1);
            }
            
            if(FD_ISSET(sd,&readfds)){
                process1.push_back(thread(cust_recv,sd,readfds));   
            }
            for (std::thread &t: process1) {
            if (t.joinable()) {
                t.join();
                
        }
        }    
        }
        if(sendcount1 == C.im_neighbours && recvcount1 == C.im_neighbours){
            break;
        }
        sleep(1);
    }

    return 0; 
}
