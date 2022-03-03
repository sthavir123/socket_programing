#include<sys/socket.h>
#include<bits/stdc++.h>
#include<filesystem>
#include<dirent.h>
using namespace std;


# define port 8080



int main(int argc, char* argv[]){
// getting data   
    
    if(argc!=3){
        cout<<"Please provide 2 argumnets"<<endl;
        return 0;
    }
    ifstream indata;
    string config = argv[1]; 
    string dir_path = argv[2];


    // DATA From Config File
    int client_id;
    int in_port;
    int unique_private_id;
    int im_neighbours;
    vector<pair<int,int>> neighbours;
    int no_files;
    vector<string> files;

    // Directory contents
    vector<string> myfiles;

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
    iss0 >> client_id >> in_port >> unique_private_id;

    istringstream iss1(lines[1]);
    iss1 >> im_neighbours;

    istringstream iss2(lines[2]);
    for(int  i = 0; i< im_neighbours;i++){
        int a, b;
        iss2>>a>>b;
        neighbours.push_back(make_pair(a,b));
    }
    istringstream iss3(lines[3]);
    iss3>>no_files;
    for(int i = 0; i < no_files;i++){
        files.push_back(lines[4+i]);
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
                myfiles.push_back(string(d->d_name));
            }
        }
        closedir(dr);
    }
    else
        cout<<"\nError Occurred!"<<endl;
    
    
    // Establishing Connections
        


    return 0; 
}
