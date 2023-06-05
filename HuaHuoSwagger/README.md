The purpose of this project is to generate the TypeScript-Axios client of the HuaHuo backend APIs.
It will consume the swagger.json and uses gradle-swagger-generator-plugin to do so. (https://github.com/int128/gradle-swagger-generator-plugin)
- You can get the json from executing "gradlew generateOpenApiDocs" in the HuahuoBackend/app folder. This task is added by spring-doc to auto generate OpenAPI json from Java code.
- You can also access the swagger UI from here: http://localhost:8080/swagger-ui/#/
- Note that whenever HuaHuo backend API is changed, regenerate the ts-axios client of this project.
- TODO: How to automatically update this client??

TODOs:
1. Fix all UNDO-REDO related bugs.
2. Tutorials.
3. Improve of UI.
4. Gif load issue.