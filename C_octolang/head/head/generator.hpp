#pragma once

#include <concepts>
#include <cstring>

template <class T, typename R>
concept generator = requires (T t) {
	{ t.next() } -> std::same_as<R>;
	{ t.done() } -> std::same_as<bool>; //fuck coroutines
};
template <class T, typename R>
concept closeable_generator = generator<T,R> && requires (T t) {
	{ t.close() } -> std::same_as<void>;
};
template <typename R>
class Closeable_Generator_Ptr final {
	int* placeholder;
	bool closed;
public:
	virtual R next() {
		R* r = (R*)std::malloc(sizeof(R));
		std::memset(r, 0, sizeof(R));
		return *r;
	};
	virtual bool done() { return true; };
	virtual void close() {};
	bool isopen() {
		return !closed && placeholder != nullptr;
	}
	Closeable_Generator_Ptr& operator=(Closeable_Generator_Ptr&) = default;
	virtual Closeable_Generator_Ptr& operator=(std::nullptr_t) { return *this; }
};
template <typename R>
class Generator_Ptr final {
	int& placeholder;
	bool closed;
public:
	virtual R next() {
		R* r = (R*)std::malloc(sizeof(R));
		std::memset(r, 0, sizeof(R));
	};
	virtual bool done() { return true; };
	virtual void close() {};
	Generator_Ptr& operator=(Generator_Ptr&) = default;
	virtual Generator_Ptr& operator=(std::nullptr_t) { return *this; }
};
template <typename T,typename R> 
class _Generator_Ptr {
protected:
	T* t;
	bool closed = false;
public:
	virtual R next() {
		return t->next();
	}
	virtual bool done() {
		return t->done();
	}
	_Generator_Ptr(T* t) noexcept : t(t) {};
	virtual void close() {}
	virtual _Generator_Ptr<T,R>& operator=(std::nullptr_t) {
		t = nullptr;
		return *this;
	}
	operator Generator_Ptr<R>&() {
		return *reinterpret_cast<Generator_Ptr<R>*>(this);
	}
};
template <typename T,typename R> //cant be used as a member/variable
class _Closeable_Generator_Ptr : public _Generator_Ptr<T,R> {
public:
	virtual void close() override {
		this->t->close();
		this->closed = true;
	}
	operator Closeable_Generator_Ptr<R>&() {
		return *reinterpret_cast<Closeable_Generator_Ptr<R>*>(this);
	};
	_Closeable_Generator_Ptr(T* t) noexcept : _Generator_Ptr<T,R>(t) {};
	virtual _Closeable_Generator_Ptr<T,R>& operator=(std::nullptr_t) override {
		if (!this->closed) {
			if (this->t != nullptr) {
				this->t->close();
			}
		}
		this->t = nullptr;
		return *this;
	}
};
template <class T, typename R>
Generator_Ptr<R> to_gen_ptr(T* t) requires generator<T, R> {
	return _Generator_Ptr<T, R>(t);
};
template <class T, typename R>
Closeable_Generator_Ptr<R> to_cgen_ptr(T* t) requires closeable_generator<T, R> {
	return _Closeable_Generator_Ptr<T, R>(t);
};