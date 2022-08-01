package online.huahuo.backend.security;

import lombok.Builder;
import lombok.Getter;
import org.springframework.security.web.authentication.WebAuthenticationDetails;
import org.springframework.security.web.authentication.preauth.PreAuthenticatedAuthenticationToken;

@Getter
public class JwtPreAuthenticationToken extends PreAuthenticatedAuthenticationToken {
    private static final long serialVersionUID = -7187761086212129292L;

    @Builder
    public JwtPreAuthenticationToken(JwtUserDetails principal, WebAuthenticationDetails details) {
        super(principal, null, principal.getAuthorities());
        super.setDetails(details);
    }

    @Override
    public Object getCredentials() {
        return null;
    }
}
