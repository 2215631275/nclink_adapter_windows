如果cmake编译过程中出现"operate[]不匹配"的bug，在源代码中添加
std::string s_id = idData["id"];
然后将jsonData[]中内容替换为s_id