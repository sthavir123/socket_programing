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
#include<sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <openssl/md5.h>

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
string dir_path;

unsigned char result[MD5_DIGEST_LENGTH];
mutex mtx;
// Print the MD5 sum as hex-digits.
void print_md5_sum(unsigned char* md) {
    int i;
    for(i=0; i <MD5_DIGEST_LENGTH; i++) {
            printf("%02x",md[i]);
    }
}

// Get the size of the file by its file descriptor
unsigned long get_size_by_fd(int fd) {
    struct stat statbuf;
    if(fstat(fd, &statbuf) < 0) exit(-1);
    return statbuf.st_size;
}

void get_md5(string filepath){
    int file_descript;
    unsigned long file_size;
    char* file_buffer;

    file_descript = open(filepath.c_str(), O_RDONLY);
    if(file_descript < 0) exit(-1);

    file_size = get_size_by_fd(file_descript);
   
    file_buffer = static_cast<char*>( mmap(0, file_size, PROT_READ, MAP_SHARED, file_descript, 0));
    MD5((unsigned char*) file_buffer, file_size, result);
    munmap(file_buffer, file_size); 

    print_md5_sum(result);
    //printf("  %s\n", filepath);

}


long int getfileSize(string filepath){
    ifstream in_file(filepath, ios::binary);
    in_file.seekg(0, ios::end);
    long int file_size = in_file.tellg();
    return file_size;
}

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
    
    map<string,int> socketmap1;
    map<string,int> socketmap2;
    map<int,bool>recieved_all;
    map<string ,int> sendto;
    vector<tuple<string,string,int>> tothis;
    vector<tuple<string,string,int>> fromthis;
    map<string,vector<pair<string,int>>> bhejna;
    map<string,vector<pair<string,int>>> lena;
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





void download_file(int sd,string filepath,int fz){
    int n  = fz;
    filepath = "./"+filepath;
    //cout<<filepath<<endl;
    
    ofstream download;
    download.open(filepath,ios::trunc|ios::out);
    char buffer[1024];
    while(n>0){
        int val_read = read(sd,buffer,1024);
        //cout<<buffer<<endl;
        download.write(buffer,val_read);
        n = n - val_read;
    }
    download.close();
    
    sleep(2);
    string mess="phase3.3;";
    if(send(sd,mess.c_str(),mess.length(),0)<0){
        perror("send");
    }
    
    
}

void upload_file(int sd,string filepath,int fz){
    char buffer[1024];
    int file_fd = open(filepath.c_str(),0);
    if(file_fd==-1) cout<<"ERR...Opening file at: "<<filepath<<endl;
    int sent = sendfile(sd,file_fd,0,fz);
    if(sent<0){
        perror("sent");
    }
    
    char bufferx[1024];
    while(true){
    int valread  = read(sd,bufferx,1024);
    if(valread<0){
        perror("read");
    }
    else{
        std::istringstream iss(bufferx);
                    std::string token;
                    
                    vector<string> temp2;
                    while (std::getline(iss, token, ';'))
                    {
                        temp2.push_back(token);
                    }
        //cout<<temp2[0]<<endl;
        //string s = bufferx;
        
        if(temp2[0]=="phase3.3"){
            break;
        }
    }
    
    }
}

void prot1(string u_id,vector<pair<string,int>> file_info1,vector<pair<string,int>> file_info2){
    if(file_info1.size()!=0){
        for(auto item: file_info1){
            string filename = item.first;
            int size = item.second;
            filename ="./"+filename;  
            upload_file(socketmap1[u_id],filename,size);
            sleep(1);
        }
        
    }
    else{
        for(auto item:file_info2){
            string filename = item.first;
            int size = item.second;
            //cout<<"downloading"<<filename<<endl;
            download_file(socketmap2[u_id],filename,size);
            sleep(1);
        }
        
    }
}
void prot2(string u_id,vector<pair<string,int>> file_info1,vector<pair<string,int>> file_info2){
    if(unique_private_id > stoi(u_id)){
       
        for(auto item: file_info1){
            string filename = item.first;
            int size = item.second;
            filename ="./"+filename;  
            upload_file(socketmap1[u_id],filename,size);
            sleep(1);
        }
        
        for(auto item:file_info2){
            string filename = item.first;
            int size = item.second;
            //cout<<"uplaoding"<<filename;
            download_file(socketmap2[u_id],filename,size);
            sleep(1);
        }
    }
    else{
        for(auto item:file_info2){
            string filename = item.first;
            int size = item.second;
            //cout<<"uplaoding"<<filename;
            download_file(socketmap2[u_id],filename,size);
            sleep(1);
        }
        for(auto item: file_info1){
            string filename = item.first;
            int size = item.second;
            filename ="./"+filename;  
            upload_file(socketmap1[u_id],filename,size);
            sleep(1);
        }

    }
}
void cust_recv(int sd){
    
                int valread,addrlen;
                struct sockaddr_in address;
                char localbuff[1024];
                valread = read( sd , localbuff, 1024);
                
                //localbuff = buffer2;
                if (valread == 0)
                {
                    //Somebody disconnected , get his details and print
                    //getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    close( sd );
                }
                
                //Echo back the message that came in
                
        else{
            if(localbuff[0]!='\0'){
                    //cout<<localbuff<<endl;

                    std::istringstream iss(localbuff);
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
                        //cout<<"sockmap1HERE"<<sd<<endl;
                        socketmap1[temp[2]] = sd;   
                    }

                    
                    }
                    for(int i=0;i<1024;i++){localbuff[i] = '\0';}
                    
                }
            
}

void cust_send2(int sd,string mess){
    
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
    char localbuff[1024];
    for(int i=0;i<1024;i++){localbuff[i] = '\0';}
        int valread,addrlen;
        struct sockaddr_in address;
        valread = read( sd , localbuff, 1024);
        
        if(valread == 0){
            close(sd);
        }
        else if(localbuff[0]!='\0'){
                    //cout<<"this buffer"<<localbuff<<endl;

                    std::istringstream iss(localbuff);
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

                            
                            cust_send2(sd,mess);
                        }
                        else if(temp[0]=="phase2.1"){
                            //cout<<"socketmap2HERE"<<sd<<endl;
                            socketmap2[unique_id[temp[1]]] = sd;
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
                        else if(temp[0]=="phase3"){
                            //cout<<"recv"<<endl;
                            if(temp[1]=="0"){
                                string mess="phase3.1:0;";
                                //cout<<mess<<endl;
                                cust_send2(sd,mess);
                            }
                            else{
                                string uni_id = temp[1];
                                temp.erase(temp.begin());
                                temp.erase(temp.begin());
                                string mess = "phase3.1:"+to_string(unique_private_id);
                                for(auto item: temp){
                                    string filepath = dir_path+item;
                                    int size = getfileSize(filepath);
                                    mess+=":"+item+":"+to_string(size);

                                    //string mess = "phase3.1:"+to_string(unique_private_id)+":"+ temp[2]+":"+to_string(size)+";";
                                    tothis.push_back(make_tuple(uni_id,filepath,size));
                                }
                                mess+=";";
                                //cout<<"mess: "<<mess<<endl;
                                cust_send2(sd,mess);
                                //sleep(1);
                                   
                                //upload_file(socketmap1[temp[1]],dir_path,size);
                                
                            }        
                        }
                        else if(temp[0]=="phase3.1"){
                            
                            if(temp[1]!="0"){
                                string u_id = temp[1];
                                temp.erase(temp.begin());
                                temp.erase(temp.begin());
                                
                                int i = 0;
                                while(i<temp.size()){
                                //cout<<socketmap2[temp[1]]<<" "<<socketmap1[temp[1]]<<endl;
                                    string file_path = dir_path+"Downloaded/"+temp[i];
                                    //cout<<"This to file "<<temp[i]<<endl;
                                    i+=1;
                                //cout<<file_path<<endl;
                                    //cout<<"This to int "<<temp[i]<<endl;
                                    fromthis.push_back(make_tuple(u_id,file_path,stoi(temp[i])));
                                    i+=1;
                                }
                            }
                        }
                        
                        
                    }
                   
                    checked++;
                    
             
            
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
    dir_path = argv[2];
    string dirname = "./"+dir_path+"/Downloaded";
    int check = mkdir(dirname.c_str(),0777);
    cout<<check<<endl;
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
    
    for(auto item:C.files){
        string temp= item;
        if (!temp.empty() && temp[temp.size() - 1] == '\r')
                    {temp.erase(temp.size() - 1);}
        
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
    string mess = "phase1:"+to_string(C.client_id) +":" +to_string(C.unique_private_id)+ ":" + to_string(C.in_port)+";";
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

   
    vector<thread> process1;
    vector<thread> process2;
    sleep(2);
    while(true){
        
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
    
    for(int i =0 ;i < C.im_neighbours;i++){
        sd = client_socket[i];
        //process1.push_back(thread(&Client::cust_send2,&C,sd,mess1));
        C.cust_send2(sd,mess1);
    }
    for(int i = 0;i<C.im_neighbours;i++){
        C.recieved_all[sd]=false;
    }
    for(int i =0 ;i < C.im_neighbours;i++){
        sd = client_in[i];
        //process1.push_back(thread(&Client::cust_recv2,&C,sd));
        C.cust_recv2(sd);
    }
    //phase2.1
    for(int i = 0; i < C.im_neighbours;i++){
        sd = client_socket[i];
        if(!C.recieved_all[sd]){
            //cout<<"recieved2"<<endl;
            C.cust_recv2(sd);
            
        }
     }
    
    for(auto it = C.filelocation.begin();it!=C.filelocation.end();it++){
        
        int depth;
        if((*it).second == 0){
            depth=0;
        }
        else{
            depth=1;
        }
        string temp = (*it).first;
        if (!temp.empty() && temp[temp.size() - 1] == '\r')
                    {temp.erase(temp.size() - 1);}
        
        printf("Found %s",temp.c_str());
        printf(" at %d with MD5 0 at depth %d\n",(*it).second,depth);
       
    }
    sleep(3);

    //phase3

    map<int,bool>sendto;
    for(auto item: C.unique_id){
        sendto[C.socketmap1[item.second]] = false;
    }
    map<int,vector<string>> appendfile;
    for(auto item: C.filelocation){
        vector<string>empty;
        if(item.second!=0){
            appendfile[item.second]=empty;
        }
    }
    for(auto item: C.filelocation){
        if(item.second!=0){
            appendfile[item.second].push_back(item.first);
            
        }
    }
    for(auto item:appendfile){
        sendto[C.socketmap1[to_string(item.first)]]=true;
        //string mess3 = "phase3:"+to_string(C.unique_private_id)+":"+item.first+";";
            string mess3 = "phase3:"+to_string(C.unique_private_id);
            for(auto it:item.second){
                mess3+=":"+it;
            }
            mess3+=";";
            //cout<<"Sent this:"<<mess3<<endl;
            C.cust_send2(C.socketmap1[to_string(item.first)],mess3);
    }
    // for(auto item : C.filelocation){
    //     if(item.second!=0){
    //         sendto[C.socketmap1[to_string(item.second)]]=true;
    //         string mess3 = "phase3:"+to_string(C.unique_private_id)+":"+item.first+";";
    //         cout<<"Sent this:"<<mess<<endl;
    //         C.cust_send2(C.socketmap1[to_string(item.second)],mess3);            
    //     }
        
    // }
    for(auto item: sendto){
        if(!item.second){
            string mess3 = "phase3:0:0;";

            C.cust_send2(item.first,mess3);
        }
    }

    
    for(int i = 0;i<C.im_neighbours;i++){
        int sd = client_socket[i];
        process1.push_back(thread(&Client::cust_recv2,&C,sd));
        //cout<<"here";
    }

    
    
    sleep(3);
    for(int i = 0; i < C.im_neighbours ;i++){
        int sd = client_in[i];
        process1.push_back(thread(&Client::cust_recv2,&C,sd));
    }

    for (std::thread &t: process1) {
            if (t.joinable()) {
                t.join();
            }
    }

    sleep(10);
    //cout<<"REACHED HERE"<<endl;
    map<string,bool> bool1;
    map<string,bool> bool2;
    for(auto item: C.tothis){
        vector<pair<string,int>> empty;
        C.bhejna[get<0>(item)] = empty;
        bool1[get<0>(item)] = false;
        bool2[get<0>(item)] = false;
    }
    for(auto item: C.fromthis){
        vector<pair<string,int>> empty;
        C.lena[get<0>(item)] = empty;
        bool1[get<0>(item)] = false;
        bool2[get<0>(item)] = false;
    }
    for(auto item: C.tothis){
       
        C.bhejna[get<0>(item)].push_back(make_pair(get<1>(item),get<2>(item)));
        
        bool1[get<0>(item)] = true;

    }
    for(auto item: C.fromthis){
        C.lena[get<0>(item)].push_back(make_pair(get<1>(item),get<2>(item)));
        bool2[get<0>(item)] = true;
    }
    
    
   
    for(auto item: bool1){
        //cout<<item.first<<" "<<item.second<<" "<< bool2[item.first];
        if(item.second!=bool2[item.first]){
            process1.push_back(thread(&Client::prot1,&C,item.first,C.bhejna[item.first],C.lena[item.first]));
            //C.prot1(item.first,C.bhejna[item.first],C.lena[item.first]);
        }
        else{
            process1.push_back(thread(&Client::prot2,&C,item.first,C.bhejna[item.first],C.lena[item.first]));
        }
    }

    
    // for(auto item : C.tothis){
    //     string u_id = get<0>(item);
    //     string filename = get<1>(item);
    //     int size = get<2>(item);
    //     FILE* pFile;
    //     filename ="./"+filename;  
    //     pFile = fopen (filename.c_str() , "r");
    //     process1.push_back(thread(&Client::upload_file,&C,C.socketmap1[u_id],filename,size));
    //     //process1.push_back(thread(&Client::send_file,&C,pFile,C.socketmap1[u_id]));
    //     //C.upload_file(C.socketmap1[u_id],filename,size);
    // }

    
    
    // for(auto item : C.fromthis){
    //     string u_id = get<0>(item);
    //     string filename = get<1>(item);
    //     int size = get<2>(item);
    //     cout<<"uplaoding"<<filename;
    //     process1.push_back(thread(&Client::download_file,&C,C.socketmap2[u_id],filename,size));
    //     //C.download_file(C.socketmap2[u_id],filename,size);
    // }
    for (std::thread &t: process1) {
            if (t.joinable()) {
                t.join();
            }
    }
    cout<<"exiting"<<endl;
    sleep(5);
    for(auto item:C.filelocation){
        if(item.second!=0){
            string filename = "./"+dir_path+"Downloaded/"+item.first;
            cout<<"Found "<<item.first<<" with MD5 ";
            get_md5(filename);
            cout<<"at depth 0"<<endl;
        }
        else{
            cout<<"Found "<<item.first<<" with MD5 0 at depth 0"<<endl;
            
        }
    }
    
    return 0; 
}

//phase 4

