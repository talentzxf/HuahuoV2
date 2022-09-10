package online.huahuo.backend.controller;


import lombok.AllArgsConstructor;
import online.huahuo.backend.db.ProjectFileDB;
import online.huahuo.backend.storage.StorageService;
import org.springframework.core.io.ByteArrayResource;
import org.springframework.core.io.Resource;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;

@Controller
@AllArgsConstructor
public class FileDownloadController {
    private final StorageService storageService;

    @GetMapping("/projects/{projectId}")
    public ResponseEntity<Resource> downloadProject(@PathVariable Long projectId) throws IOException {
        ProjectFileDB fileDB = storageService.getById(projectId);

        String fullPath = fileDB.getFullPath();
        byte[] fileData = Files.readAllBytes(Paths.get(fullPath));

        ByteArrayResource resource = new ByteArrayResource(fileData);

        HttpHeaders headers = new HttpHeaders();
        headers.add(HttpHeaders.CONTENT_DISPOSITION, "attachment;filename=\"" + fileDB.getName() + "\"");
        headers.add("X-Suggested-Filename", fileDB.getName());

        return ResponseEntity.ok()
                .headers(headers)
                .contentLength(fileData.length)
                .contentType(MediaType.APPLICATION_OCTET_STREAM)
                .body(resource);
    }

    @GetMapping("/projects")
    public ResponseEntity<List<ProjectFileDB>> listProjects(@PathVariable Long pageNumber, @PathVariable Long pageSize){

    }
}
