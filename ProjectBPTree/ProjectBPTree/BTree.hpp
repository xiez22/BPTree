#include "utility.hpp"
#include <functional>
#include <cstddef>
#include "exception.hpp"
#include <cstdio>
namespace sjtu {
	template <class Key, class Value, class Compare = std::less<Key> >
	class BTree {
	private:
		// Your private members go here
		//常量静态参数
		//B+树大数据块大小
		constexpr static size_t BLOCK_SIZE = 8192;
		//大数据块预留数据块大小
		constexpr static size_t INIT_SIZE = 128;
		//大数据块能够存储孩子的个数(M)
		constexpr static size_t BLOCK_KEY_NUM = (BLOCK_SIZE - INIT_SIZE + sizeof(Key)) / (sizeof(Key) + sizeof(size_t));
		//小数据块能够存放的记录的个数(L)
		constexpr static size_t BLOCK_PAIR_NUM = (BLOCK_SIZE - INIT_SIZE) / sizeof(pair<Key, Value>);
		//B+树索引存储地址
		constexpr static char BPTREE_ADDRESS[128] = "E:/Test/BPTree/bptree_data.sjtu";
		//内存管理文件存储地址
		constexpr static char MEMORY_ADDRESS[128] = "E:/Test/BPTree/memory_data.sjtu";

		//私有类
		//B+树文件头
		class File_Head {
		public:
			//存储BLOCK占用的空间
			size_t block_cnt = 1;
			//存储根节点的位置
			size_t root_pos = 0;
			//存储数据块头
			size_t data_block_head = 0;
			//存储数据块尾
			size_t data_block_rear = 0;
			//存储大小
			size_t _size = 0;
		};

		//块头
		class Block_Head {
		public:
			//存储类型(0普通 1叶子)
			bool block_type = false;
			//数量（L或者M）
			size_t _size = 0;
			//相对位置
			size_t _pos = 0;
			//父结点
			size_t _parent = 0;
			//上一个结点
			size_t _last = 0;
			//下一个结点
			size_t _next = 0;
		};

		//私有变量
		//文件头
		File_Head tree_data;

		//私有函数
		//块内存写入
		template <class MEM_TYPE>
		static void mem_read(MEM_TYPE buff, size_t buff_size, size_t pos, FILE* fp) {
			fseek(fp, buff_size * pos, SEEK_SET);
			fread(buff, buff_size, 1, fp);
		}

		//块内存读取
		template <class MEM_TYPE>
		static void mem_write(MEM_TYPE buff, size_t buff_size, size_t pos, FILE* fp) {
			fseek(fp, buff_size * pos, SEEK_SET);
			fwrite(buff, buff_size, 1, fp);
			fflush(fp);
		}

		//写入B+树基本数据
		void write_tree_data() {
			auto fp = fopen(BPTREE_ADDRESS, "r+");
			fseek(fp, 0, SEEK_SET);
			char buff[BLOCK_SIZE] = { 0 };
			memcpy(buff, &tree_data, sizeof(tree_data));
			mem_write(buff, BLOCK_SIZE, 0, fp);
			fclose(fp);
		}

		//获取新内存
		size_t memory_allocation() {
			auto fp = fopen(MEMORY_ADDRESS, "r+");
			size_t memory_remain;
			fseek(fp, 0, SEEK_SET);
			fread(&memory_remain, sizeof(size_t), 1, fp);
			if (memory_remain == 0) {
				++tree_data.block_cnt;
				write_tree_data();
				auto fp_tree = fopen(BPTREE_ADDRESS, "r+");
				char buff[BLOCK_SIZE] = { 0 };
				mem_write(buff, BLOCK_SIZE, tree_data.block_cnt, fp_tree);
				fclose(fp_tree);
				fclose(fp);
				return tree_data.block_cnt;
			}
			size_t cur_pos;
			fseek(fp, memory_remain * sizeof(size_t), SEEK_SET);
			fread(&cur_pos, sizeof(size_t), 1, fp);
			--memory_remain;
			fseek(fp, 0, SEEK_SET);
			fwrite(&memory_remain, sizeof(size_t), 1, fp);
			fflush(fp);
			fclose(fp);
			return cur_pos;
		}

		//删除旧内存
		void memory_free(size_t pos) {
			auto fp = fopen(MEMORY_ADDRESS, "r+");
			size_t memory_remain;
			fseek(fp, 0, SEEK_SET);
			fread(&memory_remain, sizeof(size_t), 1, fp);
			++memory_remain;
			fseek(fp, memory_remain * sizeof(size_t), SEEK_SET);
			fwrite(&pos, sizeof(size_t), 1, fp);
			fseek(fp, 0, SEEK_SET);
			fwrite(&memory_remain, sizeof(size_t), 1, fp);
			fflush(fp);
			fclose(fp);
		}

	public:
		typedef pair<const Key, Value> value_type;

		class const_iterator;
		class iterator {
		private:
			// Your private members go here
			//存储当前
			BTree* cur_bptree = nullptr;
			//存储当前所处的块
			size_t cur_block = 0;
			//存储当前指向的元素位置
			size_t cur_pos = 0;
			//存储当前块的元素个数以加快移动速度
			size_t block_size = 0;

		public:
			iterator() {
				// TODO Default Constructor
			}
			iterator(const iterator& other) {
				// TODO Copy Constructor
				cur_bptree = other.cur_bptree;
				cur_block = other.cur_block;
				cur_pos = other.cur_pos;
				block_size = other.block_size;
			}
			// Return a new iterator which points to the n-next elements
			iterator operator++(int) {
				// Todo iterator++
				auto temp = *this;
				++cur_pos;
				if (cur_pos >= block_size) {
					//读取数据
					auto fp = fopen(BPTREE_ADDRESS, "r+");
					char buff[BLOCK_SIZE] = { 0 };
					mem_read(buff, BLOCK_SIZE, cur_block, fp);
					Block_Head temp;
					memcpy(&temp, buff, sizeof(temp));
					auto next_block = temp._next;
					memset(buff, 0, sizeof(buff));
					mem_read(buff, BLOCK_SIZE, next_block, fp);
					memcpy(&temp, buff, sizeof(temp));
					cur_block = temp._pos;
					cur_pos = 0;
					block_size = temp._size;
					fclose(fp);
				}
				return temp;
			}
			iterator& operator++() {
				// Todo ++iterator
				++cur_pos;
				if (cur_pos >= block_size) {
					//读取数据
					auto fp = fopen(BPTREE_ADDRESS, "r+");
					char buff[BLOCK_SIZE] = { 0 };
					mem_read(buff, BLOCK_SIZE, cur_block, fp);
					Block_Head temp;
					memcpy(&temp, buff, sizeof(temp));
					auto next_block = temp._next;
					memset(buff, 0, sizeof(buff));
					mem_read(buff, BLOCK_SIZE, next_block, fp);
					memcpy(&temp, buff, sizeof(temp));
					cur_block = temp._pos;
					cur_pos = 0;
					block_size = temp._size;
					fclose(fp);
				}
				return *this;
			}
			iterator operator--(int) {
				// Todo iterator--
				auto temp = *this;
				--cur_pos;
				if (cur_pos < 0) {
					//读取数据
					auto fp = fopen(BPTREE_ADDRESS, "r+");
					char buff[BLOCK_SIZE] = { 0 };
					mem_read(buff, BLOCK_SIZE, cur_block, fp);
					Block_Head temp;
					memcpy(&temp, buff, sizeof(temp));
					auto next_block = temp._last;
					memset(buff, 0, sizeof(buff));
					mem_read(buff, BLOCK_SIZE, next_block, fp);
					memcpy(&temp, buff, sizeof(temp));
					cur_block = temp._pos;
					cur_pos = 0;
					block_size = temp._size;
					fclose(fp);
				}
				return temp;
			}
			iterator& operator--() {
				// Todo --iterator
				--cur_pos;
				if (cur_pos < 0) {
					//读取数据
					auto fp = fopen(BPTREE_ADDRESS, "r+");
					char buff[BLOCK_SIZE] = { 0 };
					mem_read(buff, BLOCK_SIZE, cur_block, fp);
					Block_Head temp;
					memcpy(&temp, buff, sizeof(temp));
					auto next_block = temp._last;
					memset(buff, 0, sizeof(buff));
					mem_read(buff, BLOCK_SIZE, next_block, fp);
					memcpy(&temp, buff, sizeof(temp));
					cur_block = temp._pos;
					cur_pos = 0;
					block_size = temp._size;
					fclose(fp);
				}
				return *this;
			}
			// Overloaded of operator '==' and '!='
			// Check whether the iterators are same
			value_type& operator*() const {
				// Todo operator*, return the <K,V> of iterator
				if (cur_pos >= block_size)
					throw invalid_iterator();
				auto fp = fopen(BPTREE_ADDRESS, "r+");
				char buff[BLOCK_SIZE] = { 0 };
				mem_read(buff, BLOCK_SIZE, cur_block, fp);
				value_type result;
				memcpy(&result, buff + INIT_SIZE + cur_pos * sizeof(value_type), sizeof(value_type));
				
			}
			bool operator==(const iterator& rhs) const {
				// Todo operator ==
			}
			bool operator==(const const_iterator& rhs) const {
				// Todo operator ==
			}
			bool operator!=(const iterator& rhs) const {
				// Todo operator !=
			}
			bool operator!=(const const_iterator& rhs) const {
				// Todo operator !=
			}
			value_type* operator->() const noexcept {
				/**
				 * for the support of it->first.
				 * See
				 * <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/>
				 * for help.
				 */
			}
		};
		class const_iterator {
			// it should has similar member method as iterator.
			//  and it should be able to construct from an iterator.
		private:
			// Your private members go here
		public:
			const_iterator() {
				// TODO
			}
			const_iterator(const const_iterator& other) {
				// TODO
			}
			const_iterator(const iterator& other) {
				// TODO
			}
			// And other methods in iterator, please fill by yourself.
		};
		// Default Constructor and Copy Constructor
		BTree() {
			// Todo Default
			auto fp = fopen(BPTREE_ADDRESS, "r+");
			if (!fp) {
				//创建新的树
				auto fp_tree = fopen(BPTREE_ADDRESS, "w+");
				auto fp_memory = fopen(MEMORY_ADDRESS, "w+");
				fseek(fp_memory, 0, SEEK_SET);
				size_t temp_size = 0;
				fwrite(&temp_size, sizeof(size_t), 1, fp_memory);
				fflush(fp_memory);
				char buff[BLOCK_SIZE] = { 0 };
				memcpy(buff, &tree_data, sizeof(tree_data));
				mem_write(buff, BLOCK_SIZE, 0, fp_tree);

				auto new_block = memory_allocation();
				auto head_pos = memory_allocation();
				auto rear_pos = memory_allocation();
				tree_data.root_pos = new_block;
				tree_data.data_block_head = head_pos;
				tree_data.data_block_rear = rear_pos;
				write_tree_data();

				//写入叶子数据
				Block_Head temp_block;
				temp_block.block_type = true;
				temp_block._pos = tree_data.root_pos;
				temp_block._last = tree_data.data_block_head;
				temp_block._next = tree_data.data_block_rear;
				memset(buff, 0, sizeof(buff));
				memcpy(buff, &temp_block,sizeof(temp_block));
				mem_write(buff, BLOCK_SIZE, temp_block._pos, fp_tree);

				temp_block._pos = tree_data.data_block_head;
				temp_block._last = 0;
				temp_block._next = tree_data.root_pos;
				memset(buff, 0, sizeof(buff));
				memcpy(buff, &temp_block, sizeof(temp_block));
				mem_write(buff, BLOCK_SIZE, temp_block._pos, fp_tree);

				temp_block._pos = tree_data.data_block_rear;
				temp_block._last = tree_data.root_pos;
				temp_block._next = 0;
				memset(buff, 0, sizeof(buff));
				memcpy(buff, &temp_block, sizeof(temp_block));
				mem_write(buff, BLOCK_SIZE, temp_block._pos, fp_tree);

				fclose(fp_tree);
				fclose(fp_memory);
				return;
			}
			char buff[BLOCK_SIZE] = { 0 };
			mem_read(buff, BLOCK_SIZE, 0, fp);
			memcpy(&tree_data, buff, sizeof(tree_data));
			fclose(fp);
		}
		BTree(const BTree& other) {
			// Todo Copy
			tree_data.block_cnt = other.tree_data.block_cnt;
			tree_data.data_block_head = other.tree_data.data_block_head;
			tree_data.data_block_rear = other.tree_data.data_block_rear;
			tree_data.root_pos = other.tree_data.root_pos;
			tree_data._size = other.tree_data._size;
		}
		BTree& operator=(const BTree& other) {
			// Todo Assignment
			tree_data.block_cnt = other.tree_data.block_cnt;
			tree_data.data_block_head = other.tree_data.data_block_head;
			tree_data.data_block_rear = other.tree_data.data_block_rear;
			tree_data.root_pos = other.tree_data.root_pos;
			tree_data._size = other.tree_data._size;
			return *this;
		}
		~BTree() {
			// Todo Destructor
		}
		// Insert: Insert certain Key-Value into the database
		// Return a pair, the first of the pair is the iterator point to the new
		// element, the second of the pair is Success if it is successfully inserted
		pair<iterator, OperationResult> insert(const Key& key, const Value& value) {
			// TODO insert function
		}
		// Erase: Erase the Key-Value
		// Return Success if it is successfully erased
		// Return Fail if the key doesn't exist in the database
		OperationResult erase(const Key& key) {
			// TODO erase function
			return Fail;  // If you can't finish erase part, just remaining here.
		}
		// Overloaded of []
		// Access Specified Element
		// return a reference to the first value that is mapped to a key equivalent to
		// key. Perform an insertion if such key does not exist.
		Value& operator[](const Key& key) {}
		// Overloaded of const []
		// Access Specified Element
		// return a reference to the first value that is mapped to a key equivalent to
		// key. Throw an exception if the key does not exist.
		const Value& operator[](const Key& key) const {}
		// Access Specified Element
		// return a reference to the first value that is mapped to a key equivalent to
		// key. Throw an exception if the key does not exist
		Value& at(const Key& key) {}
		// Overloaded of const []
		// Access Specified Element
		// return a reference to the first value that is mapped to a key equivalent to
		// key. Throw an exception if the key does not exist.
		const Value& at(const Key& key) const {}
		// Return a iterator to the beginning
		iterator begin() {}
		const_iterator cbegin() const {}
		// Return a iterator to the end(the next element after the last)
		iterator end() {}
		const_iterator cend() const {}
		// Check whether this BTree is empty
		bool empty() const {
			return tree_data._size == 0;
		}
		// Return the number of <K,V> pairs
		size_t size() const {
			return tree_data._size;
		}
		// Clear the BTree
		void clear() {}
		/**
		 * Returns the number of elements with key
		 *   that compares equivalent to the specified argument,
		 * The default method of check the equivalence is !(a < b || b > a)
		 */
		size_t count(const Key& key) const {}
		/**
		 * Finds an element with key equivalent to key.
		 * key value of the element to search for.
		 * Iterator to an element with key equivalent to key.
		 *   If no such element is found, past-the-end (see end()) iterator is
		 * returned.
		 */
		iterator find(const Key& key) {}
		const_iterator find(const Key& key) const {}
	};
}  // namespace sjtu