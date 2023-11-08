package online.huahuo.backend.controller;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.ResponseEntity;
import org.springframework.security.access.prepost.PreAuthorize;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

@Slf4j
@Controller
@AllArgsConstructor
public class EventGraphController {
    @ResponseBody
    @PreAuthorize("hasRole('READER')")
    @GetMapping("/eventgraphs")
    public ResponseEntity listEventGraphs(@RequestParam(defaultValue = "0") int pageNumber,
                                          @RequestParam(defaultValue = "10") int pageSize) {
        return ResponseEntity.ok("Not Implemented!");
    }
}
