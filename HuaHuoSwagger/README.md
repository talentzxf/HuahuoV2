The purpose of this project is to generate the TypeScript-Axios client of the HuaHuo backend APIs.
It will consume the swagger.json and uses gradle-swagger-generator-plugin to do so. (https://github.com/int128/gradle-swagger-generator-plugin)
- You can get the json from the backend API: http://localhost:8080/v2/api-docs
- You can also access the swagger UI from here: http://localhost:8080/swagger-ui/#/
- Note that whenever HuaHuo backend API is changed, regenerate the ts-axios client of this project.
- TODO: How to automatically update this client??