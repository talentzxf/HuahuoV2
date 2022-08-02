package online.huahuo.backend.controller;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.RequiredArgsConstructor;
import net.bytebuddy.utility.RandomString;
import online.huahuo.backend.db.*;
import org.apache.commons.lang3.RandomStringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.authentication.AuthenticationProvider;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.web.bind.annotation.*;

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
    private final UserService userService;

    @PostMapping("/login")
    ResponseEntity<?> login(@RequestParam(required = false) String username, @RequestParam(required = false) String password){
        LoginStatus loginStatus = new LoginStatus();
        if(username == null || password == null){
            loginStatus.setFailReason("Username or pwd is null");
            loginStatus.setHttpStatus(HttpStatus.BAD_REQUEST);
            return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
        }

        loginStatus.setUserName(username);

        UserDB user = userRepository.findByUsername(username);
        if(user == null){
            loginStatus.setFailReason("Can't find user!");
            loginStatus.setHttpStatus(HttpStatus.BAD_REQUEST);
            return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
        }

        if(user.getStatus() != UserStatus.ACTIVE){
            loginStatus.setFailReason("User is not active!");
            return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
        }

        UsernamePasswordAuthenticationToken credentials = new UsernamePasswordAuthenticationToken(username, password);
        SecurityContextHolder.getContext().setAuthentication(authenticationProvider.authenticate(credentials));

        loginStatus.setHttpStatus(HttpStatus.OK);
        return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
    }
}
