package online.huahuo.backend.controller;

import lombok.Data;
import lombok.RequiredArgsConstructor;
import online.huahuo.backend.db.UserDB;
import online.huahuo.backend.db.UserRepository;
import online.huahuo.backend.db.UserStatus;
import online.huahuo.backend.exception.UserNotFoundException;
import online.huahuo.backend.security.JwtTokenUtil;
import online.huahuo.backend.security.SecurityProperties;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.authentication.AuthenticationProvider;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.Date;
import java.util.Optional;

@Data
class LoginStatus{
    private String userName;
    private String failReason;
    private String jwtToken;
    private HttpStatus httpStatus;
}

@Data
class TokenValidResponse{
    private String userName;
    private Boolean isValid;
}

@RestController
@RequiredArgsConstructor
public class LoginController {
    private final UserRepository userRepository;
    private final AuthenticationProvider authenticationProvider;
    private final SecurityProperties properties;
    private final JwtTokenUtil jwtTokenUtil;

    @Transactional
    public String issueToken(Long userId) {
        Optional<UserDB> userOptional = userRepository.findById(userId);
        if (userOptional.isEmpty()) throw new UserNotFoundException(userId);

        UserDB user = userOptional.get();
        return jwtTokenUtil.generateToken(user);
    }

    @GetMapping("/tokenValid")
    ResponseEntity<?> isTokenValid(@RequestParam String userName, @RequestParam String jwtToken){
        TokenValidResponse tokenValidResponse = new TokenValidResponse();
        tokenValidResponse.setUserName(userName);
        tokenValidResponse.setIsValid(false);

        if(userRepository.findByUsername(userName) != null
        && jwtTokenUtil.getUsernameFromToken(jwtToken).equals(userName)
        && !jwtTokenUtil.isTokenExpired(jwtToken)){
            tokenValidResponse.setIsValid(true);
        }

        return new ResponseEntity<>(tokenValidResponse, HttpStatus.OK);
    }

    @PostMapping("/login")
    ResponseEntity<?> login(@RequestParam(required = false) String userName, @RequestParam(required = false) String password){
        LoginStatus loginStatus = new LoginStatus();
        if(userName == null || password == null){
            loginStatus.setFailReason("Username or pwd is null");
            loginStatus.setHttpStatus(HttpStatus.BAD_REQUEST);
            return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
        }

        loginStatus.setUserName(userName);

        UserDB user = userRepository.findByUsername(userName);
        if(user == null){
            loginStatus.setFailReason("Can't find user!");
            loginStatus.setHttpStatus(HttpStatus.BAD_REQUEST);
            return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
        }

        if(user.getStatus() != UserStatus.ACTIVE){
            loginStatus.setFailReason("User is not active!");
            return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
        }

        String token = issueToken(user.getId());

        loginStatus.setJwtToken(token);
        UsernamePasswordAuthenticationToken credentials = new UsernamePasswordAuthenticationToken(userName, password);
        SecurityContextHolder.getContext().setAuthentication(authenticationProvider.authenticate(credentials));

        loginStatus.setHttpStatus(HttpStatus.OK);

        user.setLastLoginTime(new Date());
        userRepository.save(user);

        return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
    }
}
