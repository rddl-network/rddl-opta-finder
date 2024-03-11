void cmdInterpreter(std::vector<std::string> &cmd);

enum command_code {
    eSetWifi,
    eMnemonic,
    eGetPublicKey,
    eAttestMachine,
    eNotarizeData,
    eCreateCsr,
    eStoreCert,
    eReadCert,
    eTestAWSMQTTMsg,
    NumOfUserCommand
};