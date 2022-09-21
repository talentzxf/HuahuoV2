package online.huahuo.backend.controller;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.extern.slf4j.Slf4j;
import online.huahuo.backend.db.ProjectFileDB;
import online.huahuo.backend.db.ProjectRespository;
import online.huahuo.backend.db.ProjectStatus;
import online.huahuo.backend.storage.StorageService;
import org.springframework.core.io.ByteArrayResource;
import org.springframework.core.io.Resource;
import org.springframework.data.domain.PageRequest;
import org.springframework.data.domain.Pageable;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.security.access.prepost.PreAuthorize;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.multipart.MultipartFile;
import com.google.common.io.Resources;

import java.io.IOException;
import java.net.URL;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.NoSuchAlgorithmException;
import java.util.List;

@Data
@AllArgsConstructor
class ProjectCallStatus {
    private long fileId;
    private boolean succeeded;
    private String msg;
}

@Data
class ListProjectResult {
    private int totalCount;
    private List<ProjectFileDB> projectFiles;
}

@Data
class ProjectExistResponse {
    private String projectName;
    private Boolean exist;
}

@Slf4j
@Controller
@AllArgsConstructor
public class ProjectController {
    private final StorageService storageService;
    @javax.annotation.Resource
    private ProjectRespository projectRespository;

    @ResponseBody
    @PostMapping("/projects/projectData")
    public ProjectCallStatus handleFileUpload(@RequestParam("file") MultipartFile file, @RequestParam(value = "force_override", required = false) Boolean forceOverride) throws IOException, NoSuchAlgorithmException {
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        String username = authentication.getName();

        if (forceOverride == null) { // By default, override previous project
            forceOverride = true;
        }

        ProjectFileDB fileDB = storageService.store(username, file, forceOverride);
        return new ProjectCallStatus(fileDB.getId(), true, "File uploaded successfully!");
    }

    @ResponseBody
    @PutMapping("/projects/{projectId}/description")
    public ProjectCallStatus handleChangeProjectDescription(@PathVariable("projectId") Long projectId, @RequestBody String description) {
        ProjectFileDB fileDB = storageService.getById(projectId);
        fileDB.setDescription(description);
        storageService.save(fileDB);

        return new ProjectCallStatus(fileDB.getId(), true, "Description changed successfully");
    }


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

    @PreAuthorize("hasRole('READER')")
    @GetMapping("/projects")
    public ResponseEntity<ListProjectResult> listProjects(@RequestParam(defaultValue = "0") int pageNumber, @RequestParam(defaultValue = "10") int pageSize) {
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        String username = authentication.getName();

        Pageable pageable = PageRequest.of(pageNumber, pageSize);
        List<ProjectFileDB> resultList = projectRespository.findByCreatedByAndStatus(username, ProjectStatus.ACTIVE, pageable);

        int totalProjectCount = projectRespository.countByCreatedByAndStatus(username, ProjectStatus.ACTIVE);
        ListProjectResult listProjectResult = new ListProjectResult();
        listProjectResult.setProjectFiles(resultList);
        listProjectResult.setTotalCount(totalProjectCount);
        return ResponseEntity.ok()
                .body(listProjectResult);
    }

    @PreAuthorize("hasRole('READER')")
    @GetMapping("/projects/exist")
    ResponseEntity<ProjectExistResponse> exist(@RequestParam String projectName) {
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        String username = authentication.getName();

        ProjectFileDB projectFileDB = projectRespository.findByCreatedByAndName(username, projectName);

        ProjectExistResponse response = new ProjectExistResponse();
        response.setProjectName(projectName);

        if (projectFileDB != null) {
            response.setExist(true);
        } else {
            response.setExist(false);
        }
        return new ResponseEntity<>(response, HttpStatus.OK);
    }

    @ResponseBody
    @PostMapping("/projects/{projectId}/coverPage")
    public ProjectCallStatus handleCoverPageUpload(@RequestParam("file") MultipartFile coverPageFile, @PathVariable("projectId") Long projectId) throws IOException {
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        String username = authentication.getName();

        storageService.storeCoverPage(username, projectId, coverPageFile);
        return new ProjectCallStatus(projectId, true, "Project cover page uploaded successfully!");
    }

    @GetMapping("/projects/pageNotFound")
    public ResponseEntity<Resource> coverPageNotFound() throws IOException {
        URL url = Resources.getResource("image-not-found-icon.png");
        byte[] fileData = Resources.toByteArray(url);
        ByteArrayResource resource = new ByteArrayResource(fileData);

        HttpHeaders headers = new HttpHeaders();
        headers.add(HttpHeaders.CONTENT_DISPOSITION, "attachment;filename=image-not-found-icon.png");

        return ResponseEntity.ok()
                .headers(headers)
                .contentLength(fileData.length)
                .contentType(MediaType.APPLICATION_OCTET_STREAM)
                .body(resource);
    }

    @PreAuthorize("hasRole('CREATOR')")
    @DeleteMapping("/projects/{projectId}")
    @ResponseStatus(HttpStatus.OK)
    public ResponseEntity deleteProject(@PathVariable Long projectId) throws IOException {
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        String username = authentication.getName();

        ProjectFileDB projectFileDB = projectRespository.findById(projectId).get();
        if(!projectFileDB.getCreatedBy().equals(username)){
            return ResponseEntity.badRequest().body("Project creator mismatch!");
        }

        // Delete all the project files in the folder.
        String projectDataPath = projectFileDB.getFullPath();
        String projectImgPath = projectFileDB.getCoverPagePath();

        Files.delete(Path.of(projectDataPath));
        Files.delete(Path.of(projectImgPath));

        projectRespository.deleteById(projectId);

        return ResponseEntity.ok("Project successfully deleted!");
    }

    // TODO: Add Auth!!!
    // @PreAuthorize("hasRole('READER')")
    @GetMapping("/projects/{projectId}/coverPage")
    public ResponseEntity projectCoverpage(@PathVariable Long projectId) throws IOException {
        try {
            ProjectFileDB fileDB = storageService.getById(projectId);
            String fullPath = fileDB.getCoverPagePath();
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
        } catch (Exception e) {
            log.error("Exception happened, redirect to not found page.", e);
            HttpHeaders headers = new HttpHeaders();
            headers.add("Location", "/projects/pageNotFound");

            // 302 jump to not found image
            return new ResponseEntity<byte[]>(null, headers, HttpStatus.FOUND);
        }
    }

}
