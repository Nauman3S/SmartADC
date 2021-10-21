
#define RXD2 16
#define TXD2 17
String dataV = "";
String tempValue = "";
String DataToSend = "";
void sendData_UVCommander(String d){
    DataToSend=d;
}
int checkCommand(String d, String c)
{
    int vv = d.indexOf(c);
    return vv;
}

void pollAnswer(String a)
{
    Serial2.println(a);
}

void sendToUVCommander()
{
    if (DataToSend.length() > 0)
    {
        pollAnswer(DataToSend);
        
    }
    else
    {
        pollAnswer("ALIVE OK");
    }
}

void setupCommsHandler()
{
    Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
}
void checkAndPublishData(String topic, String msg)
{
    deviceExisits=1;
    if (deviceExisits == 1)
    {
        mqttPublish(topic, msg);
    }
}
uint8_t UVCommanderPollHandler()
{
    if (Serial2.available())
    {
        dataV = Serial2.readString();
        if(dataV.length()<1){
            return 0;
        }
        if (checkCommand(dataV, "ALIVE") >= 0)
        {
            //pollAnswer("ALIVE OK");
            sendToUVCommander();
            return 1;
        }

        if (checkCommand(dataV, "HELLO") >= 0)
        {
            pollAnswer("OK - SERIAL");
            return 1;
        }

        if (checkCommand(dataV, "BLOCK") >= 0)
        {
            DataToSend="";
            return 1;
        }

        if (checkCommand(dataV, "Message") >= 0)
        {
            DataToSend="";
            return 1;
        }

        if (checkCommand(dataV, "PAYMENT") >= 0)
        {
            DataToSend="";
            return 1;
        }
        if (checkCommand(dataV, "NO INTERNET") >= 0)
        {
            DataToSend="";
            return 1;
        }
        if (checkCommand(dataV, "START") >= 0)
        {
            tempValue = ss.StringSeparator(dataV, ' ', 1);
            
            checkAndPublishData("tanning-device/updateStartSession", ss.getMacAddress() + String(";") + tempValue);
            pollAnswer(String("START ") + tempValue);
            pollAnswer(String("START ") + ss.getMacAddress() + String(";") + tempValue);
            return 1;
        }

        if (checkCommand(dataV, "STOP") >= 0)
        {
            tempValue = ss.StringSeparator(dataV, ' ', 1);
            checkAndPublishData("tanning-device/updateStopSession", ss.getMacAddress() + String(";") + tempValue);
            return 1;
        }

        if (checkCommand(dataV, "TEMP") >= 0)
        {
            tempValue = ss.StringSeparator(dataV, ' ', 1);
            checkAndPublishData("tanning-device/updateTemp", ss.getMacAddress() + String(";") + tempValue);
            return 1;
        }

        if (checkCommand(dataV, "FILTER") >= 0)
        {
            tempValue = ss.StringSeparator(dataV, ' ', 1);
            checkAndPublishData("tanning-device/updateFilter", ss.getMacAddress() + String(";") + tempValue);
            return 1;
        }

        if (checkCommand(dataV, "LAMP") >= 0)
        {
            tempValue = ss.StringSeparator(dataV, ' ', 1);
            checkAndPublishData("tanning-device/updateLamp", ss.getMacAddress() + String(";") + tempValue);
            return 1;
        }

        if (checkCommand(dataV, "YEAR") >= 0)
        {
            tempValue = ss.StringSeparator(dataV, ' ', 1);
            checkAndPublishData("tanning-device/updateYear", ss.getMacAddress() + String(";") + tempValue);
            return 1;
        }

        if (checkCommand(dataV, "AMP") >= 0)
        {
            tempValue = ss.StringSeparator(dataV, ' ', 1);
            checkAndPublishData("tanning-device/updateAmp", ss.getMacAddress() + String(";") + tempValue);
            return 1;
        }

        if (checkCommand(dataV, "VENT") >= 0)
        {
            tempValue = ss.StringSeparator(dataV, ' ', 1);
            checkAndPublishData("tanning-device/updateVent", ss.getMacAddress() + String(";") + tempValue);
            return 1;
        }

        if (checkCommand(dataV, "VOLT") >= 0)
        {
            tempValue = ss.StringSeparator(dataV, ' ', 1);
            checkAndPublishData("tanning-device/updateVolt", ss.getMacAddress() + String(";") + tempValue);
            return 1;
        }

        if (checkCommand(dataV, "RST") >= 0)
        {
            tempValue = ss.StringSeparator(dataV, ' ', 1);
            checkAndPublishData("tanning-device/updateRst", ss.getMacAddress() + String(";") + tempValue);
            return 1;
        }
    }
}