void cmdInterpreter(std::vector<std::string> &cmd);

enum command_code {
    eHelp,
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