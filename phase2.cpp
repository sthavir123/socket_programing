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
#include <thread>
#include <mutex>
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
int filesfound;
int checked;
int connectedto;
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
    map<string,string> unique_id;
    map<string,int> filelocation;
    vector<pair<string,vector<int>>> fileloc2;
    map<int,bool>recieved_all;
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
        //cout<<"sent "<<mess<<endl;
    }        
    
    }

void cust_recv(int sd){
    
     
            
                int valread,addrlen;
                struct sockaddr_in address;
            
                valread = read( sd , buffer2, 1024);
                
                if (valread == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    close( sd );
                }
                
                //Echo back the message that came in
                
        else{
            if(buffer2[0]!='\0'){
                    //cout<<buffer2<<endl;

                    std::istringstream iss(buffer2);
                    std::string token;
                    vector<string> temp;
                    vector<string> temp2;
                    while (std::getline(iss, token, ';'))
                    {
                        temp2.push_back(token);
                    }
                    
                    std::istringstream iss1(temp2[0]);
                        
                    while (std::getline(iss1, token, ':'))
                    {
                        temp.push_back(token);
                    }
                    
                    if(temp[0]=="phase1"){
                        cout<<"Connected to "<< temp[1]<<" with unique id "<<temp[2]<<" on port "<<temp[3]<<endl;
                        unique_id[temp[1]]=temp[2];    
                    }

                    
                    }
                    for(int i=0;i<1024;i++){buffer2[i] = '\0';}
                    
                }
            
}

void cust_send2(int sd,string mess){
    //string mess = to_string(client_id);
        //for(auto item:myfiles){
            
            //mess+=":"+item;
        //}
        if( send(sd, mess.c_str(),mess.length(), 0) != mess.length() ) 
        {
            perror("send");
        }
}

void cust_recv2(int sd){
    
    int on = 0;
    if (ioctl(sd, FIONBIO, &on) < 0) {
               perror("ioctl F_SETFL, FNDELAY");
               //exit(1);
          }
    
        int valread,addrlen;
        struct sockaddr_in address;
        valread = read( sd , buffer2, 1024);
        if(valread == 0){
            close(sd);
        }
        else if(buffer2[0]!='\0'){
                    //cout<<buffer2<<endl;

                    std::istringstream iss(buffer2);
                    std::string token;
                    
                    
                    vector<string> temp2;
                    while (std::getline(iss, token, ';'))
                    {
                        token.erase( std::remove(token.begin(), token.end(), '\r'), token.end() );
                        temp2.push_back(token);
                    }
                    
                    for(auto item:temp2){
                    std::istringstream iss1(temp2[0]);
                    vector<string> temp;    
                    while (std::getline(iss1, token, ':'))
                    {
                        temp.push_back(token);
                    }
                    
                    if(temp[0]=="phase2"){
                        temp.erase(temp.begin());
                        temp.erase(temp.begin());
                        //cout<<temp[1]<<endl;
                        string mess = "phase2.1:"+to_string(client_id);
                            for(auto token:temp){
                                string file = token;
                                for(auto item:myfiles){
                                    if(file==item){
                                        mess+=":"+item;
                                    }
                                }
                            }
                            mess+=";";
                            //cout<<"mess sent "<<mess<<endl;
                            cust_send2(sd,mess);
                        }
                        else if(temp[0]=="phase2.1"){
                            string recvid = temp[1];
                            temp.erase(temp.begin());
                            temp.erase(temp.begin());
                            //cout<< recvid<<endl;
                            int un_id = stoi(unique_id[recvid]);

                            for(auto token:temp){
                        
                                string file = token;
                        
                                for(auto item:files){
                                    if(file==item){
                                        if(filelocation[file]==0){
                                            filelocation[file] = un_id;
                                            filesfound++;
                                    
                                        }
                                        else if(filelocation[file] > un_id){
                                            filelocation[file] = un_id;
                                        }
                                    }
                                }
                            }
                            recieved_all[sd]=true;    
                        }
                    }
                    //cout<<temp2.size()<<endl;
                    
                    //cout<<(token==NULL)<<endl;
                    checked++;
                    for(int i=0;i<1024;i++){buffer2[i] = '\0';}
             
            
        }
    
}

};

fd_set setfd(int master_socket,Client* C,int& max_sd,int client_in[],int client_socket[]){
    fd_set readfds;
    FD_SET(master_socket, &readfds);
        if(master_socket > max_sd){
            max_sd  = master_socket;
        }
        
        int sd;
        for(int i = 0; i< C->im_neighbours; i++){
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
        for(int i = 0; i< C->im_neighbours; i++){
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
return readfds;
}

int main(int argc, char* argv[]){
//initailisations
sendcount1=0;
recvcount1=0;
filesfound=0;
checked=0;
connectedto=0;
int connected_form=0;

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
                string temp = d->d_name;
                if (!temp.empty() && temp[temp.size() - 1] == '\r')
                    {temp.erase(temp.size() - 1);}


                C.myfiles.push_back(temp);
            }
        }
        closedir(dr);
    }
    else {cout<<"\nError Occurred!"<<endl;}
    sort(C.myfiles.begin(),C.myfiles.end());
    for(auto item: C.myfiles){
        cout<<item<<endl;
    }
    
    string mess1 = "phase2:"+to_string(C.client_id);
    for(auto item:C.files){
        mess1+=":" + item;
    }
    mess1+=";";
    
    // initialising all connected to false
    for(auto item:C.neighbours){
        C.connected[item.first] = false;
    }
    
    //initialising filesloc 
    //vector<int>empty;
    for(auto item:C.files){
        string temp= item;
        if (!temp.empty() && temp[temp.size() - 1] == '\r')
                    {temp.erase(temp.size() - 1);}
        //C.fileloc2.push_back(make_pair(item,empty));
        C.filelocation[temp]=0;
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
    string mess = "phase1:"+to_string(C.client_id) +":" +to_string(C.unique_private_id)+ ":" + to_string(C.in_port);
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
    sleep(5);
    while(true){
        //MAXTRY--;
        //process1.clear();

        //clear the socket set
        FD_ZERO(&readfds);
        max_sd=0;
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
                connectedto++;
            }
            tpi++;

        }


        //add master socket to set
        FD_SET(master_socket, &readfds);
        if(master_socket > max_sd){
            max_sd  = master_socket;
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
            
            C.cus_send(new_socket,readfds,mess);
            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++) 
            {
                //if position is empty
				if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    connected_form++;
                    break;
                }
            }

        }
        //else its some IO operation on some other socket :)
        //for (i = 0; i < C.im_neighbours; i++) 
        //{
        //    
        //    sd = client_in[i];
            
        //    if(!(sd>0)) continue;
        //  if (ioctl(sd, FIONBIO, &on) < 0) {
        //        perror("ioctl F_SETFL, FNDELAY");
        //        exit(1);
        //    }
            
        //    if(FD_ISSET(sd,&readfds)){
                //process1.push_back(thread(&Client::cust_recv,&C,sd,readfds));   
        //    }
                
        //}
                                                      
            
        // for (std::thread &t: process1) {
        //     if (t.joinable()) {
        //         t.join();
                
        // }
        // }
        //Found bar.pdf at 4526 with MD5 0 at depth 1
        if(connectedto == C.im_neighbours&&connected_form==C.im_neighbours){
            sleep(1);
            break;
        }
        sleep(1);
    }

//phase1 messages
    for(int i = 0;i<C.im_neighbours;i++){
        sd = client_in[i];
        C.cust_recv(sd);
    }
    sleep(5);

//phase 2
    //FD_ZERO(&readfds);
    //readfds = setfd(master_socket,&C,max_sd,client_in,client_socket);
        // wait for an activity with time out
    //    activity = select(max_sd+1,&readfds,NULL,NULL,NULL);
        //cout<<activity;
    //    if ((activity < 0) && (errno!=EINTR)) 
    //    {
    //        printf("select error");
    //        return 0;
    //    }

    
    for(int i =0 ;i < C.im_neighbours;i++){
        sd = client_socket[i];
        //process1.push_back(thread(&Client::cust_send2,&C,sd,mess1));
        C.cust_send2(sd,mess1);
    }
    for(int i = 0;i<C.im_neighbours;i++){
        C.recieved_all[sd]=0;
    }
    for(int i =0 ;i < C.im_neighbours;i++){
        sd = client_in[i];
        //process1.push_back(thread(&Client::cust_recv2,&C,sd));
        C.cust_recv2(sd);
    }
    for(int i = 0; i < C.im_neighbours;i++){
        sd = client_socket[i];
        //if(!C.recieved_all[sd]){
            //cout<<"recieved2"<<endl;
            C.cust_recv2(sd);
            
        //}
     }
    //for (std::thread &t: process1) {
    //        if (t.joinable()) {
    //            t.join();
    //            
    //    }
    //    }
    // //Found bar.pdf at 4526 with MD5 0 at depth 1
    
    for(auto it = C.filelocation.begin();it!=C.filelocation.end();it++){
        
        int depth;
        if((*it).second == 0){
            depth=0;
            //cout<<"Found "<<item.first<<" at "<<item.second<<" with MD5 0 at depth 0"<<endl;
        }
        else{
            depth=1;
        }
        string temp = (*it).first;
        if (!temp.empty() && temp[temp.size() - 1] == '\r')
                    {temp.erase(temp.size() - 1);}
        //cout<<"Found "<<item.first;
        printf("Found %s",temp.c_str());
        printf(" at %d with MD5 0 at depth %d\n",(*it).second,depth);
        //cout<<"Found "<<item.first<<" at "<<item.second<<" with MD5 0 at depth "<<depth<<endl;
        
        
    }
    sleep(3);
    return 0; 
}
