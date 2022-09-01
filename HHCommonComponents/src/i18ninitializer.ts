import i18next from 'i18next'
import HttpApi from 'i18next-http-backend';
import LanguageDetector from 'i18next-browser-languagedetector';

i18next
    .use(HttpApi)
    .use(LanguageDetector)
    .init({
        fallbackLng: 'en',
        load: "languageOnly",
        backend: {
            loadPath: '/i18n/{{lng}}-{{ns}}.json'
        }
    })