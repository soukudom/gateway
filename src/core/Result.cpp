#include <Poco/Exception.h>

#include "core/Answer.h"

using namespace BeeeOn;
using namespace Poco;

Result::Result(Poco::AutoPtr<Answer> answer, bool locked):
	m_status(Status::PENDING),
	m_answer(*answer)
{
	if (locked)
		m_answer.addResultUnlocked(this);
	else
		m_answer.addResult(this);
}

Result::~Result()
{
}

void Result::setStatus(const Result::Status status)
{
	FastMutex::ScopedLock guard(lock());
	setStatusUnlocked(status);
}

void Result::setStatusUnlocked(const Result::Status status)
{
	assureLocked();

	if (m_status == status)
		return;

	if (status.raw() < m_status.raw()) {
		throw Poco::InvalidArgumentException(
			"invalid status change: " + status);
	}

	m_status = status;
	notifyUpdated();
}

Result::Status Result::status() const
{
	FastMutex::ScopedLock guard(lock());
	return statusUnlocked();
}

Result::Status Result::statusUnlocked() const
{
	assureLocked();
	return m_status;
}

void Result::notifyUpdated()
{
	assureLocked();
	m_answer.setDirtyUnlocked(true);
	m_answer.event().set();
}

FastMutex &Result::lock() const
{
	return m_answer.lock();
}

void Result::assureLocked() const
{
	if (lock().tryLock()) {
		lock().unlock();
		throw IllegalStateException(
			"modifying unlocked Result");
	}
}

EnumHelper<BeeeOn::Result::StatusEnum::Raw>::ValueMap &Result::StatusEnum::valueMap()
{
	static EnumHelper<Result::Status::Raw>::ValueMap valueMap = {
		{Status::PENDING, "PENDING"},
		{Status::SUCCESS, "SUCCESS"},
		{Status::FAILED, "FAILED"},
	};

	return valueMap;
}
