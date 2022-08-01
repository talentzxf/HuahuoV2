package online.huahuo.backend.controller;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.RequiredArgsConstructor;
import online.huahuo.backend.db.UserDB;
import online.huahuo.backend.db.UserRepository;
import online.huahuo.backend.db.UserStatus;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.authentication.AuthenticationProvider;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

@Data
class LoginStatus{
    private String userName;
    private String failReason;
    private HttpStatus httpStatus;
}

@RestController
@RequiredArgsConstructor
public class LoginController {
    private final UserRepository userRepository;
    private final AuthenticationProvider authenticationProvider;

    @PostMapping("/login")
    ResponseEntity<?> login(@RequestParam String username, @RequestParam String password){
        LoginStatus loginStatus = new LoginStatus();
        UserDB user = userRepository.findByUsername(username);
        if(user == null){
            loginStatus.setUserName(username);
            loginStatus.setFailReason("Can't find user!");
            loginStatus.setHttpStatus(HttpStatus.BAD_REQUEST);
            return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
        }

        if(user.getStatus() != UserStatus.ACTIVE){
            loginStatus.setUserName(username);
            loginStatus.setFailReason("User is not active!");
            return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
        }

        UsernamePasswordAuthenticationToken credentials = new UsernamePasswordAuthenticationToken(username, password);
        SecurityContextHolder.getContext().setAuthentication(authenticationProvider.authenticate(credentials));

        return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
    }
}
