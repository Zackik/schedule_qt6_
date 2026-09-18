m_oauth->setModifyParametersFunction([](QAbstractOAuth::Stage stage, QVariantMap *parameters) {
    if (stage == QAbstractOAuth::Stage::RequestingAuthorization) {
        parameters->insert("access_type", "offline");
        parameters->insert("prompt", "consent");
    }
});
