package online.huahuo.backend.controller;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.extern.slf4j.Slf4j;
import online.huahuo.backend.db.BinaryFileDB;
import online.huahuo.backend.db.ElementDB;
import online.huahuo.backend.db.ElementRepository;
import online.huahuo.backend.storage.StorageService;
import org.springframework.http.ResponseEntity;
import org.springframework.security.access.prepost.PreAuthorize;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.*;

@Slf4j
@Controller
@AllArgsConstructor
public class ElementController {
    private ElementRepository elementRepository;
    private final StorageService storageService;

    @PreAuthorize("hasRole('CREATOR')")
    @ResponseBody
    @PostMapping(value = "/elements")
    public ResponseEntity<ElementDB> createElement(@RequestParam String elementId,
                                             @RequestParam Long fileId,
                                             @RequestParam(required = false, defaultValue = "true") boolean editable,
                                             @RequestParam(required = false, defaultValue = "true") boolean sharable){

        BinaryFileDB binaryFileDB = storageService.getById(fileId);
        if(binaryFileDB == null){
            return ResponseEntity.notFound().build();
        }

        ElementDB elementDB = new ElementDB();
        elementDB.setElementId(elementId);
        elementDB.setBinaryFileDB(binaryFileDB);

        elementDB = elementRepository.save(elementDB);

        return ResponseEntity.ok(elementDB);
    }

    @PreAuthorize("hasRole('CREATOR')")
    @ResponseBody
    @GetMapping(value = "/elements/{elementId}")
    public ResponseEntity<ElementDB> getElement(@PathVariable String elementId){
        ElementDB elementDB = elementRepository.getByElementId(elementId);
        if(elementDB == null){
            return ResponseEntity.notFound().build();
        }

        return ResponseEntity.ok(elementDB);
    }
}
