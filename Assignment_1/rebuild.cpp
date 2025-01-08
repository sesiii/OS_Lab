/*
Name: Dadi Sasank Kumar
Roll: 22CS10020
Assignment 1
PC NO: 42
*/

#include<iostream>
#include<fstream>
#include<cstdlib>
#include<string>
#include<cstring>
#include<cstdlib>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/wait.h>
#include<vector>
#include <sstream>


using namespace std;

//takes the foodule number and returns the dependencies of that foodule, saves it in a vector
vector<int> getDependencies(int foodule){
    ifstream infile("foodep.txt");
    string line;
    int n;

    infile >> n;
    infile.ignore();
    for(int i=0; i<foodule; i++){
        getline(infile,line);
    }

    vector<int> dependencies;
    istringstream iss(line.substr(line.find(":")+1));
    int dep;

    while(iss >> dep){
        dependencies.push_back(dep);
    }
    return dependencies;
}

//reads the visited foodules from done.txt and returns it in a vector
vector<int> readVisited(int n){
    ifstream infile("done.txt");
    vector<int> visited(n,0);
    int i;
    for(int i=0; i<n; i++){
        infile >> visited[i];
    }
    return visited;
}

// void writeVisited(vector<int> visited){
//     ofstream outfile("done.txt");
//     for(int i=0; i<visited.size(); i++){
//         outfile << visited[i] << endl;
//     }
// }

//writes the visited foodules to done.txt
void writeVisited(vector<int>& visited){
    ofstream outfile("done.txt");
    for(int i: visited){
        outfile << i <<" "; 
    }
    outfile << endl;
}

//rebuilds the foodule and its dependencies
void rebuild(int foodule, bool isRoot){
    ifstream infile("foodep.txt"); // read the number of foodules from foodep.txt
    int n;
    infile >> n;
    infile.close();

    //create a new vector of size n and initialize it to 0, if it is the root foodule and this will be in done.txt
    if(isRoot){
        vector<int> visited(n,0);
        writeVisited(visited);
    }

    //gets dependencies of the foodule in the form of a vector
    vector<int> dependencies = getDependencies(foodule);
     //reads the visited foodules from done.txt and returns it in a vector
    vector<int> visited = readVisited(n);

    for(int dep: dependencies){
        if(visited[dep-1] == 0){ // if the dependency is not visited, call a fork() and exec() to rebuild the dependency
            pid_t pid = fork();
            if(pid == 0){
                string foodule = to_string(dep);
                execl("./rebuild", "rebuild", foodule.c_str(),"child",nullptr); //execl replaces the current process with the new process 
                cerr<< "Error: exec() failed for foodule"<<dep << endl;
                exit(1);
            }
            else if(pid>0){ // else wait for the child to finish
                wait(nullptr);
            }
            else{
                cerr << "Error: fork() failed for foodule" << dep << endl;
                exit(1);
            }
        }
    }

    visited = readVisited(n); //reads the visited foodules from done.txt and returns it in a vector
    visited[foodule-1] = 1; //mark the foodule as visited
    writeVisited(visited);//writes the visited foodules to done.txt

    cout<<"foo"<<foodule<<" rebuilt"; //print the foodule that is rebuilt
    if(!dependencies.empty()){ // if the foodule has dependencies, print the dependencies
        cout<<" from foo";
        for(size_t i=0; i<dependencies.size(); i++){
            cout<<dependencies[i];
            if(i<dependencies.size()-1){
                cout<<", foo";
            }
        }
    }
    cout<<endl;

}

int main(int argc, char* argv[]){
    if(argc<2){
        cerr<<"Usage: ./rebuild <foodule> [child]"<<endl;    
        return 1;
    }

    int foodule = stoi(argv[1]);
    bool isRoot = (argc == 2);
    rebuild(foodule, isRoot);
    return 0;

}
    


