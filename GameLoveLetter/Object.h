#pragma once
class Object : public boost::enable_shared_from_this<Object> {
public:
	virtual ~Object() {}
	typedef boost::shared_ptr<Object> pointer;
};