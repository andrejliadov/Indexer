#include <iostream>
#include <sstream>
#include <array>
#include <cstring>
#include <typeinfo>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

struct Index{
	std::string word;
	int frequency;
};

void readIndexFile(std::string fileName, std::vector<Index>& indexVec){
	std::ifstream file;
	file.open(fileName);
	if(!file.is_open()){
		std::cout << "Error opening file" << std::endl;
	}
	
	std::string word, count;
	int freq;
	Index index;
	while(file.good()){
		getline(file, word, ' ');
		getline(file, count, '\n');

		//Store these in an array for further processing
		if(word != "\0"){
			freq = std::stoi(count);
			index.word = word;
			index.frequency = freq;
			indexVec.push_back(index);
		}
	}

	file.close();
}

//This function outputs the Index of a .txt to an index file
//If there are search terms, the programme must be able to reference all of the documents
void writeIndexFile(std::string fileName, std::vector<Index> indexVec){
	std::ofstream file;
	file.open(fileName);
	if(!file.is_open()){
		std::cout << "Error opening file" << std::endl;
	}

	for(int i = 0; i < indexVec.size(); i++){
		file << indexVec[i].word << ' ' << indexVec[i].frequency << '\n';
	}

	file.close();
}

void readFile(std::string fileName, std::vector<std::string>& wordVec){
	std::ifstream file;
	file.open(fileName);
	if(!file.is_open()){
		std::cout << "Error opening file" << std::endl;
	}

	std::string word;
	unsigned int loc;
	while(file.good()){
		getline(file, word, ' ');
		
		if(word.find('\n') != std::string::npos){
			std::string str0 = word.substr (0, word.find('\n'));
			std::string str1 = word.substr (word.find('\n'));
			wordVec.push_back(str0);
			wordVec.push_back(str1);
		}
		else
			wordVec.push_back(word);
	}

	file.close();
}

void binarySearch(std::vector<Index> indexVec, std::string word, unsigned int start, unsigned int end, unsigned int& loc, bool& found, int recursionDepth){
	unsigned int partition = (start + end)/2;
	recursionDepth++;

	if(word == indexVec[partition].word){
		loc = partition;
		found = true;
		return;
	}
	else if(recursionDepth >= 40){
		std::cout << "ERROR: Term not found" << std::endl;
		found = false;
		return;
	}
	else if(partition < 0 || partition > indexVec.size()){
		found = false;
		return;
	}
	else if(indexVec[partition].word < word){
		start = partition + 1;
		binarySearch(indexVec, word, start, end, loc, found, recursionDepth);
	}
	else if(indexVec[partition].word > word){
		end = partition - 1;
		binarySearch(indexVec, word, start, end, loc, found, recursionDepth);
	}
	return;
}

//This function will be used to convert capital letter and remove punctuation
void sanitiser(std::vector<std::string>& wordVec){
	//Have to make a character blacklist
	char charWhitelist[] = " ().;<>[],\n0123456789:?*!'";

	//This function also needs to change the value of uppercase letters
	//Loop through the worlist and remove the charcters
	for(unsigned int i = 0; i < wordVec.size(); i++){
		for(unsigned int j = 0; j < std::strlen(charWhitelist); j++){
			wordVec[i].erase(std::remove(wordVec[i].begin(), wordVec[i].end(), charWhitelist[j]), wordVec[i].end());
		}
		for(unsigned int k = 0; k < wordVec[i].size(); k++){
			if(wordVec[i][k] >= 'A' && wordVec[i][k] <= 'Z'){
				wordVec[i][k] = wordVec[i][k] + 0x20;
			}
		}
	}
}

std::vector<Index> makeIndex(std::vector<std::string> wordVec){
	//This has to look at all of the elements in the word array of a document and 
	//determine the cardinality of each unique word
	
	std::sort(wordVec.begin(), wordVec.end());

	Index index;
	std::vector<Index> indexVec;
	unsigned int i = 0, j = 0, count = 0;
	while(i < wordVec.size()){
		while(wordVec[i] == wordVec[j] && j < wordVec.size()){
			count++;
			j++;
		}
		if(wordVec[i] != "\0"){ 
			index.word = wordVec[i];
			index.frequency = count;
			indexVec.push_back(index);
		}
		i = j;
		count = 0;
	}
	return indexVec;
}

std::string formatFileName(std::string fileName){
	std::string indexFileName = "";
	for(unsigned int i = 0; i < fileName.find('.'); i++){
		indexFileName += fileName[i];
	}
	indexFileName += "_index.txt";
	return indexFileName;
}

int findTF(std::string searchTerm, std::string indexFileName){
	//This function needs to find the number of occurances of a search term
	std::vector<Index> indexVec;
	readIndexFile(indexFileName, indexVec);
	unsigned int loc = 0;
	bool found;

	binarySearch(indexVec, searchTerm, 0, indexVec.size() - 1, loc, found, 0);
	if(found){
		return indexVec[loc].frequency;
	}
	return -1;
}

std::string findFiles(){
	//This function checks for the presense of files in the working directory
	std::array<char, 128> buffer;
    	std::string result;
    	FILE* pipe = popen("ls", "r");
    	
	if (!pipe){
        	std::cerr << "Couldn't start command." << std::endl;
        	return 0;
    	}

	while (fgets(buffer.data(), 128, pipe) != NULL) {
        	result += buffer.data();
    	}
	return result;
}

void parseDirectory(std::string fileNames, std::string sub, std::vector<std::string>& fileVec){
	//This functions job is to make the directory readable by
	//the programme
	std::string indexFile;
	std::stringstream ss(fileNames);
	
	while(std::getline(ss, indexFile, '\n')){
		if(indexFile.find(sub) != std::string::npos){
			fileVec.push_back(indexFile);
		}
	}
}

float findIDF(std::string searchTerm, std::vector<std::string> fileVec){
	//This function needs to check that the serch term is in the file	
	std::vector<Index> indexVec;
	std::string files = findFiles();
	unsigned int loc = 0, DF = 0;
	bool found;
		
	for(unsigned int i = 0; i < fileVec.size(); i++){
		readIndexFile(fileVec[i], indexVec);
		binarySearch(indexVec, searchTerm, 0, indexVec.size() - 1, loc, found, 0);
		if(found){
			DF++;
		}
	}
	float IDF = ((float)DF/fileVec.size());
	return IDF;
}

void removeString(std::string& docName, std::string remove){
	unsigned int pos = docName.find(remove);
	if(pos != std::string::npos){
		docName.erase(pos, (docName.size()) - 1);	
	}
}

std::vector<std::string> parseDocs(std::vector<std::string> fileVec){
	std::vector<std::string> docVec;
	
	for(unsigned int i = 0; i < fileVec.size(); i++){
		if(fileVec[i].find("_index.txt") == std::string::npos){
			docVec.push_back(fileVec[i]);
		}
	}

	return docVec;
}

void checkForIndex(std::vector<std::string> docVec, std::vector<std::string> indexFileVec){
	std::vector<std::string> wordVec;
	std::vector<Index> indexVec;
	std::string command, indexFile;

	for(unsigned int i = 0; i < docVec.size(); i++){
		removeString(docVec[i], ".txt");
		indexFile = docVec[i] + "_index.txt";
		for(unsigned int j = 0; j < indexFileVec.size(); j++){
			if(indexFileVec[j].find(indexFile) == std::string::npos && j == (indexFileVec.size() - 1)){
				readFile((docVec[i] + ".txt"), wordVec);
				sanitiser(wordVec);	
				indexVec = makeIndex(wordVec);
				//Need to touch index file into the directory
				command = "touch " + indexFile;
				system(command.c_str());
				writeIndexFile(indexFile, indexVec);
			}	
		}
	}	
}

//Need to make a function that takes in the Search Terms, sanitises, looks for all of the
//Index files and calculates an IDF term. The function must look for all of the .txt files
//and check that there is not a corresponding _index.txt. If there is not a coresponding _index.txt
//file an index must be made at runtime.

struct FileStruct{
	std::string name;
	int TF;
	float TFIDF;
};

struct Term{
	std::string word;
	float IDF;
};

void findTFIDF(FileStruct* file, Term* term, unsigned int numFile, unsigned int numTerm){
	for(unsigned int i = 0; i < numFile; i++){
		for(unsigned int j = 0; j < numTerm; j++){
			file[i].TFIDF = (term[j].IDF) * ((float)file[i].TF);
		}
	}	
}

int main (int argc, char* argv[]){
	std::vector<std::string> wordVec, fileVec, docVec, indexFileVec, searchQuery;
	std::vector<Index> indexVec;

	std::string files = findFiles();

	parseDirectory(files, ".txt", fileVec);
	docVec = parseDocs(fileVec);
	//docVec contains all of the normal files without index files
		
	parseDirectory(files, "_index.txt", indexFileVec);
	checkForIndex(docVec, indexFileVec);
	files = findFiles();
	indexFileVec.clear();
	parseDirectory(files, "_index.txt", indexFileVec);
	//indexFileVec contains the names of all the index files
	
	for(unsigned int i = 1; i < argc; i++){
		searchQuery.push_back(argv[i]);
	}

	//Need to calculate the TF for all of the search terms
	std::sort(searchQuery.begin(), searchQuery.end());
	std::sort(indexFileVec.begin(), indexFileVec.end());
	std::sort(fileVec.begin(), fileVec.end());

	FileStruct* file = new FileStruct[indexFileVec.size()];
	for(unsigned int i = 0; i < indexFileVec.size(); i++){
		file[i].name = fileVec[i];
	}
	for(unsigned int i = 0; i < searchQuery.size(); i++){
		for(unsigned int j = 0; j < indexFileVec.size(); j++){
			std::cout << i << "  " << j << std::endl;
			file[j].TF = findTF(searchQuery[i], indexFileVec[j]);	
		}
	}
	
	//Need to calculate the IDF for all of the terms
	Term* term = (Term*) malloc(sizeof(Term) * searchQuery.size());
	for(unsigned int i = 0; i < searchQuery.size(); i++){
		term[i].word = searchQuery[i];
	}

	for(unsigned int i = 0; i < searchQuery.size(); i++){
		term[i].IDF = findIDF(searchQuery[i], indexFileVec);
	}

	//I must combine the TF and the IDF value to get relevance of a document
	findTFIDF(file, term, fileVec.size(), searchQuery.size());

	for(unsigned int i = 0; i < fileVec.size(); i++){
		std::cout << file[i].TFIDF << std::endl;
	}	

	//readFile(argv[1], wordVec);
	//sanitiser(wordVec);
	//indexVec = makeIndex(wordVec);
	
	//std::string indexFile = formatFileName(argv[1]);
	//writeIndexFile(indexFile, indexVec);
	//Now that we have a data structure that contains the index, we must store that in a file
	//std::vector<std::string> searchQuery;
	//for(unsigned int i = 2; i < argc; i++){
	//	searchQuery.push_back(argv[i]);
	//}
	
	//for(int unsigned i = 0; i < searchQuery.size(); i++){
	//	findIDF(searchQuery[i]);
	//}

	return 0;
}
