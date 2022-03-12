#include<sys/socket.h>
#include<bits/stdc++.h>
#include<filesystem>
#include<dirent.h>
#include <arpa/inet.h>
#include <errno.h>

#include <unistd.h>
#define CLIENT_MAX  10000
#define MAXBUF		1024
#define MAXIP		16

using namespace std;
string loopback;
char buffer[MAXBUF];



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

int SetAddress(string comp, struct sockaddr_in *Addr)
{	
    string s = "127.0.0.1:"+comp;

    char Composite[s.length()];
    for(int i = 0;i<s.length();i++){
        Composite[i] = s[i];
    }
    cout<<Composite<<endl;
    int i;
	char IPAddress[MAXIP];

	bzero(Addr, sizeof(*Addr));
	Addr->sin_family = AF_INET;
	for ( i = 0; Composite[i] != ':'  &&  Composite[i] != 0  &&  i < MAXIP; i++ )
		IPAddress[i] = Composite[i];
	IPAddress[i] = 0;
	if ( Composite[i] == ':' )
		Addr->sin_port = htons(atoi(Composite+i+1));
	else
		Addr->sin_port = 0;
	if ( *IPAddress == 0 )
	{
		Addr->sin_addr.s_addr = INADDR_ANY;
		return 0;
	}
	else
		return ( inet_aton(IPAddress, &Addr->sin_addr) == 0 );
}



// int SetAddress2(int port , struct sockaddr_in *Addr){
//     Addr->sin_family = AF_INET;
//     Addr->sin_port = htons(port);
//     return ( inet_aton(IPAddress, &Addr->sin_addr) == 0 );

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
    int sd;
	struct sockaddr_in addr;

    if ( (sd = socket(PF_INET, SOCK_DGRAM, 0)) < 0 )
	{
		perror("Socket");
		exit(errno);
	}
    //loopback = "127.0.0.1:";
    cout<<C.client_id<<" " <<C.in_port<<endl ;
    if ( SetAddress(to_string(C.in_port), &addr) != 0 )
	{
		perror("in_port");
		exit(errno);
	}
	if ( bind(sd, (struct sockaddr *)&addr, sizeof(addr)) != 0 )
	{
		perror("Bind");
		exit(errno);
	}
    cout << "Socket binded"<<endl;
    
    for (int i= 0; i< C.im_neighbours; i++ ){
        
        if ( SetAddress( to_string(C.neighbours[i].second), &addr) != 0 )
	    {
		    perror(argv[1]);
		    exit(errno);
	    }
    
        if ( connect(sd, (struct sockaddr *)&addr, sizeof(addr)) != 0 )
	    {
		    perror("Connect");
		    exit(errno);
	    }
        else{
            cout<<"Connected to "<<C.neighbours[i].first<<endl;
        }
        //sleep(1);
    }
    string mess = to_string(C.client_id) +":" +to_string(C.unique_private_id)+ ":" + to_string(C.in_port);
    
    
    
    bool est_con = false;
    std::regex regex("\\:");
    while(!est_con){
        send(sd, mess.c_str(), mess.length(), 0);
        int bytes_read;

		bzero(buffer, sizeof(buffer));
		bytes_read = recv(sd, buffer, sizeof(buffer), 0);
        if(bytes_read>0){
            printf("Msg: %s\n", buffer);
            send(sd, "ack", strlen("ack"), 0);
        // std::vector<std::string> out(
        //             std::sregex_token_iterator(mess.begin(), mess.end(), regex, -1),
        //             std::sregex_token_iterator()
        //             );
        //     for (auto &s: out) {
        //     std::cout << s << std::endl;
        sleep(1);
    }

    }
    
    
    
    return 0; 
}
