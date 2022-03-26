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


// int SetAddress(string comp, struct sockaddr_in *Addr)
// {	
//     string s = "127.0.0.1:"+comp;

//     char Composite[s.length()];
//     for(int i = 0;i<s.length();i++){
//         Composite[i] = s[i];
//     }
//     cout<<Composite<<endl;
//     int i;
// 	char IPAddress[MAXIP];

// 	bzero(Addr, sizeof(*Addr));
// 	Addr->sin_family = AF_INET;
// 	for ( i = 0; Composite[i] != ':'  &&  Composite[i] != 0  &&  i < MAXIP; i++ )
// 		IPAddress[i] = Composite[i];
// 	IPAddress[i] = 0;
// 	if ( Composite[i] == ':' )
// 		Addr->sin_port = htons(atoi(Composite+i+1));
// 	else
// 		Addr->sin_port = 0;
// 	if ( *IPAddress == 0 )
// 	{
// 		Addr->sin_addr.s_addr = INADDR_ANY;
// 		return 0;
// 	}
// 	else
// 		return ( inet_aton(IPAddress, &Addr->sin_addr) == 0 );
// }


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
    
    
    // Establishing Connections
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
    sleep(1);
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
    //if (ioctl(master_socket, FIONBIO, &on) < 0) {
    //            perror("ioctl F_SETFL, FNDELAY");
    //            exit(1);
    //        }

    for(auto item:C.neighbours){
            struct sockaddr_in address1;
            address1.sin_family = AF_INET;
            address1.sin_addr.s_addr =  INADDR_ANY;
            address1.sin_port = htons(item.second);

            if ( connect(master_socket, (struct sockaddr *)&address1, sizeof(address1)) != 0 )
	        {
		        perror("Connect");
                if(errno != EINPROGRESS) exit(errno);
	        }
            else{
                printf("request sent to %d\n", item.first);
            }
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
        //clear the socket set
        FD_ZERO(&readfds);
 
        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
        
        

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

        
        
        // wait for an activity with time out
        activity = select(max_sd+1,&readfds,NULL,NULL,NULL);
        if ((activity < 0) && (errno!=EINTR)) 
        {
            printf("select error");
        }



        if (FD_ISSET(master_socket, &readfds)) 
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
         
            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
           
            //send new connection greeting message
            if( send(new_socket, "ECHO Daemon v1.0 \r\n", strlen("ECHO Daemon v1.0 \r\n"), 0) != strlen("ECHO Daemon v1.0 \r\n") ) 
            {
                perror("send");
            }
             
            puts("Welcome message sent successfully");
             
            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++) 
            {
                //if position is empty
				if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);
					
					break;
                }
            }

            //else its some IO operation on some other socket :)
        for (i = 0; i < max_clients; i++) 
        {
            sd = client_socket[i];
             
            if (FD_ISSET( sd , &readfds)) 
            {
                //Check if it was for closing , and also read the incoming message
                if ((valread = read( sd , buffer1, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                     
                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }
                 
                //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end of the data read
                    buffer1[valread] = '\0';
                    send(sd , buffer1 , strlen(buffer1) , 0 );
                }
            }
        }

        }
    }

    // Create a listening TCP socket
    //listenfd = socket(AF_INET, SOCK_STREAM, 0);
   // bzero(&servaddr, sizeof(servaddr));
    //servaddr.sin_family = AF_INET;
    //servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //servaddr.sin_port = htons(C.in_port);

    //binding server addr struct to listenfd
    //bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    //listen(listenfd,5);

    //create udp socket





    // if ( (sd = socket(PF_INET, SOCK_DGRAM, 0)) < 0 )
	// {
	// 	perror("Socket");
	// 	exit(errno);
	// }
    // //loopback = "127.0.0.1:";
    // cout<<C.client_id<<" " <<C.in_port<<endl ;
    // if ( SetAddress(to_string(C.in_port), &addr) != 0 )
	// {
	// 	perror("in_port");
	// 	exit(errno);
	// }
	// if ( bind(sd, (struct sockaddr *)&addr, sizeof(addr)) != 0 )
	// {
	// 	perror("Bind");
	// 	exit(errno);
	// }
    // cout << "Socket binded"<<endl;
    
    // for (int i= 0; i< C.im_neighbours; i++ ){
        
    //     if ( SetAddress( to_string(C.neighbours[i].second), &addr) != 0 )
	//     {
	// 	    perror(argv[1]);
	// 	    exit(errno);
	//     }
    
    //     if ( connect(sd, (struct sockaddr *)&addr, sizeof(addr)) != 0 )
	//     {
	// 	    perror("Connect");
	// 	    exit(errno);
	//     }
    //     else{
    //         cout<<"Connected to "<<C.neighbours[i].first<<endl;
    //     }
    //     //sleep(1);
    // }
    // string mess = to_string(C.client_id) +":" +to_string(C.unique_private_id)+ ":" + to_string(C.in_port);
    
    




    
    // bool est_con = false;
    // std::regex regex("\\:");
    // while(!est_con){
    //     send(sd, mess.c_str(), mess.length(), 0);
    //     int bytes_read;

	// 	bzero(buffer, sizeof(buffer));
	// 	bytes_read = recv(sd, buffer, sizeof(buffer), 0);
    //     if(bytes_read>0){
    //         printf("Msg: %s\n", buffer);
    //         send(sd, "ack", strlen("ack"), 0);
    //     // std::vector<std::string> out(
    //     //             std::sregex_token_iterator(mess.begin(), mess.end(), regex, -1),
    //     //             std::sregex_token_iterator()
    //     //             );
    //     //     for (auto &s: out) {
    //     //     std::cout << s << std::endl;
    //     sleep(1);
    // }

    // }
    
    
    
    return 0; 
}
