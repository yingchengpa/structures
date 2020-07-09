// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "datastructures.h"

void test_array()
{
    std::vector<int> arry; 

}

void test_list()
{
    std::list<int> oList;
}

void test_stack()
{
    std::stack<int> oStack;
}

void test_query()
{
    std::queue<int> oQuery;           //
    std::priority_queue<int> oPQueue; //优先队列
}

void test_hashmap()
{
    std::unordered_map<int, std::string> oHashMap;
}

void test_map()
{
    std::map<int, std::string> oMap;
}

void test_heap()
{
	std::vector<int> arry;

	arry.push_back(1);
	arry.push_back(10);
	arry.push_back(5);
	arry.push_back(8);
	arry.push_back(6);

    make_heap(arry.begin(), arry.end());

}

void test_skiplist()
{
	sl_map<int, std::string> oSkipList;
}

// gb2312的中文，不支持utf8格式
void ReplaceAndReportV2(Darts::DoubleArray &dic,std::string sText)
{
	static const std::size_t MAX_NUM_RESULTS = 16;

	for (int i = 0; i < sText.size();)
	{
		typename Darts::DoubleArray::result_pair_type results[MAX_NUM_RESULTS] = { 0 };

		std::size_t num_results = dic.commonPrefixSearch(&sText[i], results, MAX_NUM_RESULTS);

		if (num_results > 0)
		{
			int offset = results[(std::min)(num_results, MAX_NUM_RESULTS) - 1].length;
			std::fill_n(sText.begin() + i, offset, '*');
			i += offset;
		}
		else
		{
			if (sText[i] < 0)//表示中文
			{
				i += 2;
			}
			else
			{
				i++;
			}
		}
	}
}

// https://www.91dengdeng.cn/2020/07/03/%E6%95%8F%E6%84%9F%E8%AF%8D%E8%BF%87%E6%BB%A4/
void test_trieTree()
{      
    std::set<std::string> valid_keys;

    valid_keys.insert("伟大");
    valid_keys.insert("很伟大");
    valid_keys.insert("很大的伟大");

	std::vector<const char*> keys(valid_keys.size());

    int key_id = 0;
	for (std::set<std::string>::const_iterator it = valid_keys.begin(); it != valid_keys.end(); ++it, ++key_id) {
		keys[key_id] = it->c_str();
	}
    
    Darts::DoubleArray dic;

    //需要先将敏感词进行排序，才能进行build。不支持动态增加、删除敏感词
    dic.build(keys.size(), &keys[0]);

    std::string strTest = "你是个很大的伟大的人";

	ReplaceAndReportV2(dic,strTest);	// output : 你是个*******的人
}

void test_lrucache()
{
	/*
	CLRU_cache<int, std::string> oCache(100000); // 最多保留100000个name
	oCache.insert(1，"chen");
	oCache.insert(2，"li");
	oCache.setttl(2,5);	     // 设置key（2）过期时间为5s
	oCache.insert(3，"wan",10);	 // 插入数据，并在10s 后过期

	auto name = oCache.get(1);	  // 返回一个value的指针
	if(name){				          // 对象是否存在
		std::string myname = *name;	  // 获取对象的值
	}
	 */
}

int main()
{
	return 0;
}

