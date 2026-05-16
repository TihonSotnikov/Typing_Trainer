#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

/// \brief Потокобезопасная очередь с возможностью закрытия.
/// \tparam T Тип хранимых элементов.
template<typename T>
class ConcurrentQueue
{
private:
	std::queue<T>           queue_;
	mutable std::mutex      mtx_;
	std::condition_variable cv_;
	bool                    closed_ = false;

public:
	ConcurrentQueue()  = default;
	~ConcurrentQueue() = default;

	ConcurrentQueue(const ConcurrentQueue&) = delete;
	ConcurrentQueue(ConcurrentQueue&&)      = delete;

	ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;
	ConcurrentQueue& operator=(ConcurrentQueue&&)      = delete;

	/// \brief Проверяет очередь на пустоту.
	/// \return true, если очередь пуста, иначе false.
	[[nodiscard]] bool empty() const
	{
		std::scoped_lock lock(mtx_);
		return queue_.empty();
	}

	/// \brief Добавляет элемент. Игнорируется, если очередь закрыта.
	/// \tparam U Тип добавляемого значения.
	/// \param value Добавляемое значение.
	template<class U>
	void push(U&& value)
	{
		{
			std::scoped_lock lock(mtx_);
			if (closed_) return;
			queue_.push(std::forward<U>(value));
		}
		cv_.notify_one();
	}

	/// \brief Ожидает появления элемента и извлекает его.
	/// \return Извлеченный элемент или std::nullopt при закрытой и пустой очереди.
	std::optional<T> wait_and_pop()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		cv_.wait(lock, [this]() -> auto { return !queue_.empty() || closed_; });
		if (queue_.empty()) return std::nullopt;
		T value = std::move(queue_.front());
		queue_.pop();
		return value;
	}

	/// \brief Извлекает элемент без ожидания.
	/// \return Извлеченный элемент или std::nullopt, если очередь пуста.
	std::optional<T> try_pop()
	{
		std::scoped_lock lock(mtx_);
		if (queue_.empty()) return std::nullopt;
		T value = std::move(queue_.front());
		queue_.pop();
		return value;
	}

	/// \brief Закрывает очередь для добавления и пробуждает ожидающие потоки.
	void close()
	{
		{
			std::scoped_lock lock(mtx_);
			closed_ = true;
		}
		cv_.notify_all();
	}
};
