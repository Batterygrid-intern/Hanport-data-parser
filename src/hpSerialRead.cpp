void HanportMessageValidator::open_fd(std::string& filepath,std::ifstream& fd){
    fd.open(filepath,std::ios::binary);
    if(!fd.is_open()){
        throw std::runtime_error("Failed to open file: " + filepath);
    }
}
//read the file and close fd afterwards
void HanportMessageValidator::read_from_fd(std::vector<uint8_t>& data_buffer,std::ifstream& fd){
    char c;
    while(fd.get(c)){
        data_buffer.push_back(c);
    }
    fd.close();
    if(data_buffer.empty()){
        throw std::runtime_error("File is empty or could not be read");
    }
    std::cout << "Read " << data_buffer.size() << " bytes from file\n";
}