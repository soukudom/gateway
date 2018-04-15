#include <cppunit/extensions/HelperMacros.h>

#include "iqrf/IQRFJsonRequest.h"
#include "util/JsonUtil.h"

using namespace std;
using namespace Poco;

namespace BeeeOn {

class IQRFJsonMessageTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(IQRFJsonMessageTest);
	CPPUNIT_TEST(testCreateRequest);
	CPPUNIT_TEST_SUITE_END();

public:
	void testCreateRequest();

private:
	string jsonReformat(const string &json);
};

CPPUNIT_TEST_SUITE_REGISTRATION(IQRFJsonMessageTest);

string IQRFJsonMessageTest::jsonReformat(const string &json)
{
	return Dynamic::Var::toString(JsonUtil::parse(json));
}

void IQRFJsonMessageTest::testCreateRequest()
{
	IQRFJsonRequest::Ptr msg = new IQRFJsonRequest;

	msg->setMessageID("111");
	msg->setRequest("00.11.12.13.14");
	msg->setTimeout(10 * Timespan::SECONDS);

	CPPUNIT_ASSERT_EQUAL(
		jsonReformat(R"({
			"ctype": "dpa",
			"msgid": "111",
			"type": "raw",
			"request": "00.11.12.13.14",
			"response": "",
			"timeout": 10000
		})"),
		msg->toString()
	);
}

}
