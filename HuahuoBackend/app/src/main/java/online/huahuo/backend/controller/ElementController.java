package online.huahuo.backend.controller;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.extern.slf4j.Slf4j;
import org.springframework.security.access.prepost.PreAuthorize;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

@Data
@AllArgsConstructor
class ElementCallStatus {
    private String elementId;
    private boolean succeeded;
    private String msg;
}

@Slf4j
@Controller
@AllArgsConstructor
public class ElementController {
    @PreAuthorize("hasRole('CREATOR')")
    @ResponseBody
    @PostMapping(value = "/elements")
    public ElementCallStatus createElement(@RequestParam String elementId,
                                           @RequestParam int fileId){
        
    }
}
